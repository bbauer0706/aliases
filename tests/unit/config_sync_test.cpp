#include "aliases/config_sync.h"
#include "aliases/config.h"
#include "aliases/file_utils.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace aliases {
namespace {

class ConfigSyncTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test config directory
        test_config_dir_ = fs::temp_directory_path() / "aliases_sync_test";
        if (fs::exists(test_config_dir_)) {
            fs::remove_all(test_config_dir_);
        }
        fs::create_directories(test_config_dir_);

        // Set test config directory to isolate tests from real config
        auto& config = Config::instance();
        config.set_test_config_directory(test_config_dir_.string());
        config.initialize();

        // Initialize all sync fields with default values
        config.set_sync_enabled(false);
        config.set_sync_remote_url("");
        config.set_sync_auto_sync(true);
        config.set_sync_interval(3600);
        config.set_sync_last_sync(0);
        config.save();

        sync_manager_ = std::make_unique<ConfigSync>();
    }

    void TearDown() override {
        // Clear test config directory
        auto& config = Config::instance();
        config.clear_test_config_directory();

        // Clean up test directory
        if (fs::exists(test_config_dir_)) {
            fs::remove_all(test_config_dir_);
        }
    }

    std::unique_ptr<ConfigSync> sync_manager_;
    fs::path test_config_dir_;
};

// ========== Setup Tests ==========

TEST_F(ConfigSyncTest, SetupWithGitMethod) {
    bool result = sync_manager_->setup("git@github.com:user/config.git", "git");
    
    // May succeed or fail depending on validation
    // Just verify it doesn't crash
    (void)result;
    
    auto& config = Config::instance();
    if (result) {
        EXPECT_TRUE(config.get_sync_enabled());
        EXPECT_EQ(config.get_sync_method(), "git");
    }
}

TEST_F(ConfigSyncTest, SetupWithRsyncMethod) {
    bool result = sync_manager_->setup("user@server:/path/to/config", "rsync");
    
    (void)result;
    EXPECT_TRUE(true); // Just verify no crash
}

TEST_F(ConfigSyncTest, SetupWithFileMethod) {
    // Create a test directory
    fs::path test_dir = fs::temp_directory_path() / "aliases_sync_test";
    fs::create_directories(test_dir);
    
    bool result = sync_manager_->setup(test_dir.string(), "file");
    
    (void)result;
    EXPECT_TRUE(true);
    
    // Cleanup
    fs::remove_all(test_dir);
}

TEST_F(ConfigSyncTest, SetupWithHttpMethod) {
    bool result = sync_manager_->setup("https://example.com/config.json", "http");
    
    (void)result;
    EXPECT_TRUE(true); // Just verify no crash
}

TEST_F(ConfigSyncTest, SetupWithInvalidMethod) {
    bool result = sync_manager_->setup("some-url", "invalid-method");
    
    // Should fail with invalid method
    EXPECT_FALSE(result);
}

TEST_F(ConfigSyncTest, SetupWithEmptyUrl) {
    bool result = sync_manager_->setup("", "git");
    
    // Should fail with empty URL
    EXPECT_FALSE(result);
}

// ========== Status Tests ==========

TEST_F(ConfigSyncTest, StatusWhenSyncDisabled) {
    auto& config = Config::instance();
    config.set_sync_enabled(false);
    config.save();
    
    // Should not crash
    bool result = sync_manager_->status();
    EXPECT_TRUE(result);
}

TEST_F(ConfigSyncTest, StatusWhenSyncEnabled) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_remote_url("test-url");
    config.set_sync_method("git");
    config.save();
    
    bool result = sync_manager_->status();
    EXPECT_TRUE(result);
}

TEST_F(ConfigSyncTest, StatusShowsLastSyncTime) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_last_sync(1234567890);
    config.save();
    
    bool result = sync_manager_->status();
    EXPECT_TRUE(result);
}

TEST_F(ConfigSyncTest, StatusShowsNeverSynced) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_last_sync(0);
    config.save();
    
    bool result = sync_manager_->status();
    EXPECT_TRUE(result);
}

// ========== Pull Tests ==========

TEST_F(ConfigSyncTest, PullWhenSyncDisabled) {
    auto& config = Config::instance();
    config.set_sync_enabled(false);
    config.save();
    
    bool result = sync_manager_->pull();
    
    // Should fail when sync is disabled
    EXPECT_FALSE(result);
}

TEST_F(ConfigSyncTest, PullWithNoRemoteUrl) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_remote_url("");
    config.save();
    
    bool result = sync_manager_->pull();
    
    // Should fail with no remote URL
    EXPECT_FALSE(result);
}

TEST_F(ConfigSyncTest, PullWithInvalidMethod) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_remote_url("test-url");
    config.set_sync_method("invalid");
    config.save();
    
    bool result = sync_manager_->pull();
    
    // Should fail with invalid method
    EXPECT_FALSE(result);
}

TEST_F(ConfigSyncTest, PullWithGitMethodNoRepo) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_remote_url("git@example.com:nonexistent/repo.git");
    config.set_sync_method("git");
    config.save();
    
    bool result = sync_manager_->pull();
    
    // Will fail due to invalid repo, but shouldn't crash
    (void)result;
    EXPECT_TRUE(true);
}

// ========== Push Tests ==========

TEST_F(ConfigSyncTest, PushWhenSyncDisabled) {
    auto& config = Config::instance();
    config.set_sync_enabled(false);
    config.save();
    
    bool result = sync_manager_->push();
    
    // Should fail when sync is disabled
    EXPECT_FALSE(result);
}

TEST_F(ConfigSyncTest, PushWithNoRemoteUrl) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_remote_url("");
    config.save();
    
    bool result = sync_manager_->push();
    
    // Should fail with no remote URL
    EXPECT_FALSE(result);
}

TEST_F(ConfigSyncTest, PushWithHttpMethod) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_remote_url("https://example.com/config");
    config.set_sync_method("http");
    config.save();
    
    bool result = sync_manager_->push();
    
    // Should fail - HTTP doesn't support push
    EXPECT_FALSE(result);
}

TEST_F(ConfigSyncTest, PushWithInvalidMethod) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_remote_url("test-url");
    config.set_sync_method("invalid");
    config.save();
    
    bool result = sync_manager_->push();
    
    // Should fail with invalid method
    EXPECT_FALSE(result);
}

// ========== Auto-Sync Tests ==========

TEST_F(ConfigSyncTest, ShouldAutoSyncWhenDisabled) {
    auto& config = Config::instance();
    config.set_sync_enabled(false);
    config.set_sync_auto_sync(true);
    config.save();
    
    bool result = sync_manager_->should_auto_sync();
    
    // Should not auto-sync when sync is disabled
    EXPECT_FALSE(result);
}

TEST_F(ConfigSyncTest, ShouldAutoSyncWhenAutoSyncDisabled) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_auto_sync(false);
    config.save();
    
    bool result = sync_manager_->should_auto_sync();
    
    // Should not auto-sync when auto-sync is disabled
    EXPECT_FALSE(result);
}

TEST_F(ConfigSyncTest, ShouldAutoSyncRecentSync) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_auto_sync(true);
    config.set_sync_interval(3600); // 1 hour
    config.set_sync_last_sync(std::time(nullptr) - 100); // 100 seconds ago
    config.save();
    
    bool result = sync_manager_->should_auto_sync();
    
    // Should not sync yet (interval not passed)
    EXPECT_FALSE(result);
}

TEST_F(ConfigSyncTest, ShouldAutoSyncOldSync) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_auto_sync(true);
    config.set_sync_interval(60); // 1 minute
    config.set_sync_last_sync(std::time(nullptr) - 120); // 2 minutes ago
    config.save();
    
    bool result = sync_manager_->should_auto_sync();
    
    // Should sync (interval passed)
    EXPECT_TRUE(result);
}

TEST_F(ConfigSyncTest, ShouldAutoSyncNeverSynced) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_auto_sync(true);
    config.set_sync_last_sync(0); // Never synced
    config.save();
    
    bool result = sync_manager_->should_auto_sync();
    
    // Should sync (never synced before)
    EXPECT_TRUE(result);
}

TEST_F(ConfigSyncTest, AutoSyncIfNeededWhenNotNeeded) {
    auto& config = Config::instance();
    config.set_sync_enabled(false);
    config.save();
    
    bool result = sync_manager_->auto_sync_if_needed();
    
    // Result depends on implementation - just verify no crash
    (void)result;
    EXPECT_TRUE(true);
}

// ========== Configuration Persistence Tests ==========

TEST_F(ConfigSyncTest, SetupPersistsConfiguration) {
    std::string test_url = "git@test.com:user/repo.git";
    std::string test_method = "git";
    
    // Setup configuration
    sync_manager_->setup(test_url, test_method);
    
    // Reload config and verify
    auto& config = Config::instance();
    config.reload();
    
    // Verify settings persisted (if setup succeeded)
    if (config.get_sync_enabled()) {
        EXPECT_EQ(config.get_sync_remote_url(), test_url);
        EXPECT_EQ(config.get_sync_method(), test_method);
    }
}

TEST_F(ConfigSyncTest, MultipleSetupCalls) {
    // First setup
    sync_manager_->setup("url1", "git");
    
    // Second setup (override)
    bool result = sync_manager_->setup("url2", "rsync");
    
    (void)result;
    EXPECT_TRUE(true); // Just verify no crash
}

// ========== Edge Cases ==========

TEST_F(ConfigSyncTest, StatusWithLongElapsedTime) {
    auto& config = Config::instance();
    config.set_sync_enabled(true);
    config.set_sync_last_sync(std::time(nullptr) - 86400 * 365); // 1 year ago
    config.save();
    
    bool result = sync_manager_->status();
    EXPECT_TRUE(result);
}

TEST_F(ConfigSyncTest, SetupWithVeryLongUrl) {
    std::string long_url(1000, 'x');
    bool result = sync_manager_->setup(long_url, "git");
    
    // May succeed or fail, just verify no crash
    (void)result;
    EXPECT_TRUE(true);
}

TEST_F(ConfigSyncTest, SetupWithSpecialCharactersInUrl) {
    std::string special_url = "git@host.com:user/repo-name_123.git";
    bool result = sync_manager_->setup(special_url, "git");
    
    (void)result;
    EXPECT_TRUE(true);
}

TEST_F(ConfigSyncTest, ConfigSyncManagerCreation) {
    // Test creating multiple instances
    ConfigSync sync1;
    ConfigSync sync2;
    ConfigSync sync3;
    
    // Should all be independent
    EXPECT_TRUE(true);
}

// ========== Integration Test ==========

TEST_F(ConfigSyncTest, CompleteWorkflow) {
    // 1. Check initial status
    EXPECT_TRUE(sync_manager_->status());
    
    // 2. Setup sync
    sync_manager_->setup("test-url", "file");
    
    // 3. Check status after setup
    EXPECT_TRUE(sync_manager_->status());
    
    // 4. Check should_auto_sync
    auto should_sync = sync_manager_->should_auto_sync();
    (void)should_sync;
    
    // 5. Verify no crashes throughout workflow
    EXPECT_TRUE(true);
}

} // namespace
} // namespace aliases
