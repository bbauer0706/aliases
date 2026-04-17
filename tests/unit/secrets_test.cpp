#include <gtest/gtest.h>

#include "aliases/secrets_store.h"
#include "aliases/commands/secrets_cmd.h"
#include "aliases/config.h"

#include <filesystem>
#include <fstream>

// ---------------------------------------------------------------------------
// SecretsStore unit tests
// ---------------------------------------------------------------------------

class SecretsStoreTest : public ::testing::Test {
protected:
    std::string test_dir_;
    std::string store_path_;

    void SetUp() override {
        test_dir_   = "/tmp/aliases_test_" + std::to_string(getpid()) + "_secrets";
        store_path_ = test_dir_ + "/secrets.enc";
        std::filesystem::create_directories(test_dir_);
        aliases::Config::instance().set_test_config_directory(test_dir_);
        aliases::Config::instance().initialize();
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
    }
};

// ---- store_exists / new store ----

TEST_F(SecretsStoreTest, NewStoreDoesNotExistBeforeSave) {
    aliases::SecretsStore store(store_path_);
    EXPECT_FALSE(store.store_exists());
}

TEST_F(SecretsStoreTest, UnlockNewStoreSucceeds) {
    aliases::SecretsStore store(store_path_);
    auto r = store.unlock("hunter2");
    EXPECT_TRUE(r.success);
    EXPECT_TRUE(store.is_unlocked());
}

TEST_F(SecretsStoreTest, SaveCreatesFileWithRestrictedPermissions) {
    aliases::SecretsStore store(store_path_);
    ASSERT_TRUE(store.unlock("hunter2").success);
    store.set("K", "V");
    ASSERT_TRUE(store.save().success);

    EXPECT_TRUE(store.store_exists());

    struct stat st{};
    ASSERT_EQ(stat(store_path_.c_str(), &st), 0);
    // Should be readable/writable only by owner.
    EXPECT_EQ(st.st_mode & 0777, 0600u);
}

// ---- CRUD ----

TEST_F(SecretsStoreTest, SetAndGetRoundtrip) {
    aliases::SecretsStore store(store_path_);
    ASSERT_TRUE(store.unlock("pw").success);
    store.set("MY_KEY", "my_value");

    auto r = store.get("MY_KEY");
    ASSERT_TRUE(r.success);
    EXPECT_EQ(r.value, "my_value");
}

TEST_F(SecretsStoreTest, GetMissingKeyReturnsError) {
    aliases::SecretsStore store(store_path_);
    ASSERT_TRUE(store.unlock("pw").success);
    EXPECT_FALSE(store.get("NONEXISTENT").success);
}

TEST_F(SecretsStoreTest, DeleteRemovesKey) {
    aliases::SecretsStore store(store_path_);
    ASSERT_TRUE(store.unlock("pw").success);
    store.set("TMP", "val");
    ASSERT_TRUE(store.remove("TMP").success);
    EXPECT_FALSE(store.get("TMP").success);
}

TEST_F(SecretsStoreTest, DeleteMissingKeyReturnsError) {
    aliases::SecretsStore store(store_path_);
    ASSERT_TRUE(store.unlock("pw").success);
    EXPECT_FALSE(store.remove("GHOST").success);
}

TEST_F(SecretsStoreTest, ListNamesIsSortedAndCorrect) {
    aliases::SecretsStore store(store_path_);
    ASSERT_TRUE(store.unlock("pw").success);
    store.set("ZEBRA", "1");
    store.set("ALPHA", "2");
    store.set("MIDDLE", "3");

    const auto names = store.list_names();
    ASSERT_EQ(names.size(), 3u);
    EXPECT_EQ(names[0], "ALPHA");
    EXPECT_EQ(names[1], "MIDDLE");
    EXPECT_EQ(names[2], "ZEBRA");
}

TEST_F(SecretsStoreTest, GetAllReturnsAllPairs) {
    aliases::SecretsStore store(store_path_);
    ASSERT_TRUE(store.unlock("pw").success);
    store.set("A", "1");
    store.set("B", "2");

    const auto all = store.get_all();
    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all.at("A"), "1");
    EXPECT_EQ(all.at("B"), "2");
}

// ---- Persistence across unlock cycles ----

TEST_F(SecretsStoreTest, PersistenceRoundtrip) {
    {
        aliases::SecretsStore store(store_path_);
        ASSERT_TRUE(store.unlock("correct-horse").success);
        store.set("TOKEN", "abc123");
        store.set("API_KEY", "xyz789");
        ASSERT_TRUE(store.save().success);
    }

    // Re-open with the same password.
    aliases::SecretsStore store2(store_path_);
    ASSERT_TRUE(store2.unlock("correct-horse").success);

    EXPECT_EQ(store2.get("TOKEN").value, "abc123");
    EXPECT_EQ(store2.get("API_KEY").value, "xyz789");
}

TEST_F(SecretsStoreTest, WrongPasswordFailsDecryption) {
    {
        aliases::SecretsStore store(store_path_);
        ASSERT_TRUE(store.unlock("correct").success);
        store.set("X", "y");
        ASSERT_TRUE(store.save().success);
    }

    aliases::SecretsStore store2(store_path_);
    auto r = store2.unlock("wrong");
    EXPECT_FALSE(r.success);
    EXPECT_FALSE(store2.is_unlocked());
}

TEST_F(SecretsStoreTest, OverwritingSecretUpdatesValue) {
    aliases::SecretsStore store(store_path_);
    ASSERT_TRUE(store.unlock("pw").success);
    store.set("KEY", "old");
    store.set("KEY", "new");

    EXPECT_EQ(store.get("KEY").value, "new");
}

TEST_F(SecretsStoreTest, EmptyNameRejected) {
    aliases::SecretsStore store(store_path_);
    ASSERT_TRUE(store.unlock("pw").success);
    EXPECT_FALSE(store.set("", "value").success);
}

TEST_F(SecretsStoreTest, EmptyPasswordRejected) {
    aliases::SecretsStore store(store_path_);
    EXPECT_FALSE(store.unlock("").success);
}

TEST_F(SecretsStoreTest, SecretValueCanBeEmptyString) {
    aliases::SecretsStore store(store_path_);
    ASSERT_TRUE(store.unlock("pw").success);
    ASSERT_TRUE(store.set("EMPTY_VAL", "").success);
    EXPECT_EQ(store.get("EMPTY_VAL").value, "");
}

TEST_F(SecretsStoreTest, SecretValueCanContainSpecialChars) {
    aliases::SecretsStore store(store_path_);
    ASSERT_TRUE(store.unlock("pw").success);
    const std::string special = "p@$$w0rd!#%^&*()[]{}|\\\"'`~<>,./?";
    ASSERT_TRUE(store.set("SPECIAL", special).success);
    ASSERT_TRUE(store.save().success);

    aliases::SecretsStore store2(store_path_);
    ASSERT_TRUE(store2.unlock("pw").success);
    EXPECT_EQ(store2.get("SPECIAL").value, special);
}

TEST_F(SecretsStoreTest, CrudRequiresUnlock) {
    aliases::SecretsStore store(store_path_);
    // Not unlocked yet.
    EXPECT_FALSE(store.set("K", "V").success);
    EXPECT_FALSE(store.get("K").success);
    EXPECT_FALSE(store.remove("K").success);
    EXPECT_TRUE(store.list_names().empty());
    EXPECT_TRUE(store.get_all().empty());
}

TEST_F(SecretsStoreTest, PasswordRotateRoundtrip) {
    // Write secrets with old password.
    {
        aliases::SecretsStore store(store_path_);
        ASSERT_TRUE(store.unlock("old_pw").success);
        store.set("KEY1", "value1");
        store.set("KEY2", "value2");
        ASSERT_TRUE(store.save().success);
    }

    // Simulate rotate: unlock with old, re-encrypt with new.
    {
        aliases::SecretsStore old_store(store_path_);
        ASSERT_TRUE(old_store.unlock("old_pw").success);
        const auto all = old_store.get_all();

        // Remove the old file so the new store initialises fresh.
        std::filesystem::remove(store_path_);

        aliases::SecretsStore new_store(store_path_);
        ASSERT_TRUE(new_store.unlock("new_pw").success);
        for (const auto& [k, v] : all) new_store.set(k, v);
        ASSERT_TRUE(new_store.save().success);
    }

    // Old password must now fail.
    {
        aliases::SecretsStore store(store_path_);
        EXPECT_FALSE(store.unlock("old_pw").success);
    }

    // New password must succeed and data must be intact.
    {
        aliases::SecretsStore store(store_path_);
        ASSERT_TRUE(store.unlock("new_pw").success);
        EXPECT_EQ(store.get("KEY1").value, "value1");
        EXPECT_EQ(store.get("KEY2").value, "value2");
    }
}

// ---------------------------------------------------------------------------
// SecretsCmd unit tests (CLI argument parsing, exit codes)
// ---------------------------------------------------------------------------

class SecretsCmdTest : public ::testing::Test {
protected:
    std::string test_dir_;
    std::shared_ptr<aliases::ProjectMapper> mapper_;

    void SetUp() override {
        test_dir_ = "/tmp/aliases_test_" + std::to_string(getpid()) + "_secretscmd";
        std::filesystem::create_directories(test_dir_);
        aliases::Config::instance().set_test_config_directory(test_dir_);
        aliases::Config::instance().initialize();
        mapper_ = std::make_shared<aliases::ProjectMapper>();
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
    }
};

TEST_F(SecretsCmdTest, EmptyArgsReturnsUsageCode) {
    aliases::commands::SecretsCmd cmd(mapper_);
    EXPECT_EQ(cmd.execute({}), 2);
}

TEST_F(SecretsCmdTest, HelpFlagReturnsZero) {
    aliases::commands::SecretsCmd cmd(mapper_);
    EXPECT_EQ(cmd.execute({"--help"}), 0);
}

TEST_F(SecretsCmdTest, UnknownSubcommandReturnsUsageCode) {
    aliases::commands::SecretsCmd cmd(mapper_);
    EXPECT_EQ(cmd.execute({"bogus"}), 2);
}

TEST_F(SecretsCmdTest, SetWithNoNameReturnsUsageCode) {
    aliases::commands::SecretsCmd cmd(mapper_);
    EXPECT_EQ(cmd.execute({"set"}), 2);
}

TEST_F(SecretsCmdTest, GetWithNoNameReturnsUsageCode) {
    aliases::commands::SecretsCmd cmd(mapper_);
    EXPECT_EQ(cmd.execute({"get"}), 2);
}

TEST_F(SecretsCmdTest, DeleteWithNoNameReturnsUsageCode) {
    aliases::commands::SecretsCmd cmd(mapper_);
    EXPECT_EQ(cmd.execute({"delete"}), 2);
}

TEST_F(SecretsCmdTest, SetWithInvalidNameCharsReturnsError) {
    // Inject the master password via env var so no tty prompt is needed.
    setenv("ALIASES_MASTER_PASSWORD", "testpw", 1);

    aliases::commands::SecretsCmd cmd(mapper_);
    EXPECT_EQ(cmd.execute({"set", "KEY WITH SPACE", "val"}), 2);

    unsetenv("ALIASES_MASTER_PASSWORD");
}

TEST_F(SecretsCmdTest, SetAndListViaEnvVar) {
    setenv("ALIASES_MASTER_PASSWORD", "testpw", 1);

    aliases::commands::SecretsCmd cmd(mapper_);

    // set
    EXPECT_EQ(cmd.execute({"set", "MY_TOKEN", "abc123"}), 0);

    // list should exit 0
    EXPECT_EQ(cmd.execute({"list"}), 0);

    unsetenv("ALIASES_MASTER_PASSWORD");
}

TEST_F(SecretsCmdTest, LoadOnEmptyStoreReturnsZero) {
    setenv("ALIASES_MASTER_PASSWORD", "testpw", 1);

    aliases::commands::SecretsCmd cmd(mapper_);
    EXPECT_EQ(cmd.execute({"load"}), 0);

    unsetenv("ALIASES_MASTER_PASSWORD");
}

TEST_F(SecretsCmdTest, GetOnMissingStoreReturnsError) {
    setenv("ALIASES_MASTER_PASSWORD", "testpw", 1);

    aliases::commands::SecretsCmd cmd(mapper_);
    EXPECT_EQ(cmd.execute({"get", "MISSING"}), 1);

    unsetenv("ALIASES_MASTER_PASSWORD");
}

TEST_F(SecretsCmdTest, RotateMasterOnMissingStoreReturnsError) {
    setenv("ALIASES_MASTER_PASSWORD", "testpw", 1);

    aliases::commands::SecretsCmd cmd(mapper_);
    EXPECT_EQ(cmd.execute({"rotate-master"}), 1);

    unsetenv("ALIASES_MASTER_PASSWORD");
}
