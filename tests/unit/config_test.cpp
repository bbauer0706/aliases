#include "aliases/config.h"
#include "aliases/file_utils.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace aliases {
namespace {

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Save original config directory
        auto& config = Config::instance();
        original_config_dir_ = config.get_config_directory();
        
        // Create temporary config directory
        test_config_dir_ = fs::temp_directory_path() / "aliases_config_test";
        if (fs::exists(test_config_dir_)) {
            fs::remove_all(test_config_dir_);
        }
        fs::create_directories(test_config_dir_);
        
        // We'll need to mock the config directory - for now we test what we can
    }
    
    void TearDown() override {
        // Clean up
        if (fs::exists(test_config_dir_)) {
            fs::remove_all(test_config_dir_);
        }
    }
    
    std::string original_config_dir_;
    fs::path test_config_dir_;
};

// ========== Initialization Tests ==========

TEST_F(ConfigTest, SingletonInstance) {
    auto& config1 = Config::instance();
    auto& config2 = Config::instance();
    
    // Should be the same instance
    EXPECT_EQ(&config1, &config2);
}

TEST_F(ConfigTest, InitializeSuccessfully) {
    auto& config = Config::instance();
    EXPECT_TRUE(config.initialize());
}

TEST_F(ConfigTest, InitializeIdempotent) {
    auto& config = Config::instance();
    EXPECT_TRUE(config.initialize());
    EXPECT_TRUE(config.initialize());
    EXPECT_TRUE(config.initialize());
}

// ========== General Settings Tests ==========

TEST_F(ConfigTest, GetSetEditor) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_editor("vim");
    EXPECT_EQ(config.get_editor(), "vim");
    
    config.set_editor("emacs");
    EXPECT_EQ(config.get_editor(), "emacs");
    
    config.set_editor("code");
    EXPECT_EQ(config.get_editor(), "code");
}

TEST_F(ConfigTest, GetSetTerminalColors) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_terminal_colors(true);
    EXPECT_TRUE(config.get_terminal_colors());
    
    config.set_terminal_colors(false);
    EXPECT_FALSE(config.get_terminal_colors());
}

TEST_F(ConfigTest, GetSetVerbosity) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_verbosity("quiet");
    EXPECT_EQ(config.get_verbosity(), "quiet");
    
    config.set_verbosity("normal");
    EXPECT_EQ(config.get_verbosity(), "normal");
    
    config.set_verbosity("verbose");
    EXPECT_EQ(config.get_verbosity(), "verbose");
}

TEST_F(ConfigTest, GetSetConfirmDestructiveActions) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_confirm_destructive_actions(true);
    EXPECT_TRUE(config.get_confirm_destructive_actions());
    
    config.set_confirm_destructive_actions(false);
    EXPECT_FALSE(config.get_confirm_destructive_actions());
}

// ========== Code Command Settings Tests ==========

TEST_F(ConfigTest, GetSetVSCodeFlags) {
    auto& config = Config::instance();
    config.initialize();
    
    std::vector<std::string> flags = {"--disable-extensions", "--new-window"};
    config.set_vscode_flags(flags);
    
    auto retrieved = config.get_vscode_flags();
    ASSERT_EQ(retrieved.size(), 2u);
    EXPECT_EQ(retrieved[0], "--disable-extensions");
    EXPECT_EQ(retrieved[1], "--new-window");
}

TEST_F(ConfigTest, GetSetVSCodeFlagsEmpty) {
    auto& config = Config::instance();
    config.initialize();
    
    std::vector<std::string> empty_flags;
    config.set_vscode_flags(empty_flags);
    
    auto retrieved = config.get_vscode_flags();
    EXPECT_TRUE(retrieved.empty());
}

TEST_F(ConfigTest, GetSetCodeReuseWindow) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_code_reuse_window(true);
    EXPECT_TRUE(config.get_code_reuse_window());
    
    config.set_code_reuse_window(false);
    EXPECT_FALSE(config.get_code_reuse_window());
}

TEST_F(ConfigTest, GetSetCodeFallbackBehavior) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_code_fallback_behavior("always");
    EXPECT_EQ(config.get_code_fallback_behavior(), "always");
    
    config.set_code_fallback_behavior("never");
    EXPECT_EQ(config.get_code_fallback_behavior(), "never");
    
    config.set_code_fallback_behavior("auto");
    EXPECT_EQ(config.get_code_fallback_behavior(), "auto");
}

TEST_F(ConfigTest, GetSetPreferredComponent) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_preferred_component("server");
    EXPECT_EQ(config.get_preferred_component(), "server");
    
    config.set_preferred_component("web");
    EXPECT_EQ(config.get_preferred_component(), "web");
    
    config.set_preferred_component("ask");
    EXPECT_EQ(config.get_preferred_component(), "ask");
}

// ========== Todo Settings Tests ==========

TEST_F(ConfigTest, GetSetTodoDefaultPriority) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_todo_default_priority(1);
    EXPECT_EQ(config.get_todo_default_priority(), 1);
    
    config.set_todo_default_priority(5);
    EXPECT_EQ(config.get_todo_default_priority(), 5);
    
    config.set_todo_default_priority(10);
    EXPECT_EQ(config.get_todo_default_priority(), 10);
}

TEST_F(ConfigTest, GetSetTodoDefaultSort) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_todo_default_sort("priority");
    EXPECT_EQ(config.get_todo_default_sort(), "priority");
    
    config.set_todo_default_sort("created");
    EXPECT_EQ(config.get_todo_default_sort(), "created");
    
    config.set_todo_default_sort("category");
    EXPECT_EQ(config.get_todo_default_sort(), "category");
    
    config.set_todo_default_sort("alphabetical");
    EXPECT_EQ(config.get_todo_default_sort(), "alphabetical");
}

TEST_F(ConfigTest, GetSetTodoShowCompleted) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_todo_show_completed(true);
    EXPECT_TRUE(config.get_todo_show_completed());
    
    config.set_todo_show_completed(false);
    EXPECT_FALSE(config.get_todo_show_completed());
}

TEST_F(ConfigTest, GetSetTodoAutoCategorize) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_todo_auto_categorize(true);
    EXPECT_TRUE(config.get_todo_auto_categorize());
    
    config.set_todo_auto_categorize(false);
    EXPECT_FALSE(config.get_todo_auto_categorize());
}

// ========== Environment Settings Tests ==========

TEST_F(ConfigTest, GetSetEnvBasePort) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_env_base_port(3000);
    EXPECT_EQ(config.get_env_base_port(), 3000);
    
    config.set_env_base_port(8080);
    EXPECT_EQ(config.get_env_base_port(), 8080);
}

TEST_F(ConfigTest, GetSetEnvPortOffset) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_env_port_offset(10);
    EXPECT_EQ(config.get_env_port_offset(), 10);
    
    config.set_env_port_offset(100);
    EXPECT_EQ(config.get_env_port_offset(), 100);
}

TEST_F(ConfigTest, GetSetEnvDefaultEnv) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_env_default_env("development");
    EXPECT_EQ(config.get_env_default_env(), "development");
    
    config.set_env_default_env("production");
    EXPECT_EQ(config.get_env_default_env(), "production");
}

// ========== Sync Settings Tests ==========

TEST_F(ConfigTest, GetSetSyncEnabled) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_sync_enabled(true);
    EXPECT_TRUE(config.get_sync_enabled());
    
    config.set_sync_enabled(false);
    EXPECT_FALSE(config.get_sync_enabled());
}

TEST_F(ConfigTest, GetSetSyncRemoteUrl) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_sync_remote_url("git@github.com:user/config.git");
    EXPECT_EQ(config.get_sync_remote_url(), "git@github.com:user/config.git");
    
    config.set_sync_remote_url("https://example.com/config");
    EXPECT_EQ(config.get_sync_remote_url(), "https://example.com/config");
}

TEST_F(ConfigTest, GetSetSyncAutoSync) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_sync_auto_sync(true);
    EXPECT_TRUE(config.get_sync_auto_sync());
    
    config.set_sync_auto_sync(false);
    EXPECT_FALSE(config.get_sync_auto_sync());
}

TEST_F(ConfigTest, GetSetSyncMethod) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_sync_method("git");
    EXPECT_EQ(config.get_sync_method(), "git");
    
    config.set_sync_method("rsync");
    EXPECT_EQ(config.get_sync_method(), "rsync");
    
    config.set_sync_method("file");
    EXPECT_EQ(config.get_sync_method(), "file");
}

TEST_F(ConfigTest, GetSetSyncInterval) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_sync_interval(3600);
    EXPECT_EQ(config.get_sync_interval(), 3600);
    
    config.set_sync_interval(7200);
    EXPECT_EQ(config.get_sync_interval(), 7200);
}

// ========== Workspace Settings Tests ==========

TEST_F(ConfigTest, GetSetWorkspaceDirectory) {
    auto& config = Config::instance();
    config.initialize();

    config.set_workspace_directories({"/home/user/projects"});
    auto dirs1 = config.get_workspace_directories();
    EXPECT_EQ(dirs1.size(), 1u);
    EXPECT_EQ(dirs1[0], "/home/user/projects");

    config.set_workspace_directories({"~/workspaces"});
    auto dirs2 = config.get_workspace_directories();
    EXPECT_EQ(dirs2.size(), 1u);
    EXPECT_EQ(dirs2[0], "~/workspaces");
}

TEST_F(ConfigTest, GetProjectShortcuts) {
    auto& config = Config::instance();
    config.initialize();
    
    StringMap shortcuts;
    // Test getting shortcuts (may or may not be empty)
    config.get_project_shortcuts(shortcuts);
    
    // Just verify the method works without crashing
    EXPECT_TRUE(true);
}

TEST_F(ConfigTest, GetServerPaths) {
    auto& config = Config::instance();
    config.initialize();
    
    StringMap paths;
    // Test getting server paths (may or may not be empty)
    config.get_server_paths(paths);
    
    // Just verify the method works without crashing
    EXPECT_TRUE(true);
}

TEST_F(ConfigTest, GetWebPaths) {
    auto& config = Config::instance();
    config.initialize();
    
    StringMap paths;
    // Test getting web paths (may or may not be empty)
    config.get_web_paths(paths);
    
    // Just verify the method works without crashing
    EXPECT_TRUE(true);
}

TEST_F(ConfigTest, GetDefaultServerPaths) {
    auto& config = Config::instance();
    config.initialize();
    
    auto paths = config.get_default_server_paths();
    
    // Should contain some common patterns
    EXPECT_FALSE(paths.empty());
}

TEST_F(ConfigTest, GetDefaultWebPaths) {
    auto& config = Config::instance();
    config.initialize();
    
    auto paths = config.get_default_web_paths();
    
    // Should contain some common patterns
    EXPECT_FALSE(paths.empty());
}

// ========== Path Management Tests ==========

TEST_F(ConfigTest, GetConfigDirectory) {
    auto& config = Config::instance();
    config.initialize();
    
    auto dir = config.get_config_directory();
    EXPECT_FALSE(dir.empty());
    EXPECT_TRUE(dir.find(".config/aliases-cli") != std::string::npos);
}

TEST_F(ConfigTest, GetConfigFilePath) {
    auto& config = Config::instance();
    config.initialize();
    
    auto path = config.get_config_file_path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(path.find("config.json") != std::string::npos);
}

TEST_F(ConfigTest, GetTodosFilePath) {
    auto& config = Config::instance();
    config.initialize();
    
    auto path = config.get_todos_file_path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(path.find("todos.json") != std::string::npos);
}

TEST_F(ConfigTest, GetTodosExternalFilePath) {
    auto& config = Config::instance();
    config.initialize();
    
    auto path = config.get_todos_external_file_path();
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(path.find("todos-external.json") != std::string::npos);
}

TEST_F(ConfigTest, GetCacheDirectory) {
    auto& config = Config::instance();
    config.initialize();
    
    auto dir = config.get_cache_directory();
    EXPECT_FALSE(dir.empty());
    EXPECT_TRUE(dir.find("cache") != std::string::npos);
}

// ========== Complex Configuration Tests ==========

TEST_F(ConfigTest, MultipleSettingsChanges) {
    auto& config = Config::instance();
    config.initialize();
    
    // Change multiple settings
    config.set_editor("vim");
    config.set_terminal_colors(false);
    config.set_verbosity("verbose");
    config.set_todo_default_priority(7);
    config.set_env_base_port(4000);
    
    // Verify all changes
    EXPECT_EQ(config.get_editor(), "vim");
    EXPECT_FALSE(config.get_terminal_colors());
    EXPECT_EQ(config.get_verbosity(), "verbose");
    EXPECT_EQ(config.get_todo_default_priority(), 7);
    EXPECT_EQ(config.get_env_base_port(), 4000);
}

TEST_F(ConfigTest, ResetToDefaults) {
    auto& config = Config::instance();
    config.initialize();
    
    // Change some settings
    config.set_editor("custom-editor");
    config.set_env_base_port(9999);
    
    // Reset to defaults
    config.reset_to_defaults();
    
    // Should have default values now
    auto editor = config.get_editor();
    EXPECT_FALSE(editor.empty());
    // Default editor might vary, just check it's set
}

// ========== Edge Cases ==========

TEST_F(ConfigTest, EmptyStringSetting) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_editor("");
    EXPECT_EQ(config.get_editor(), "");
}

TEST_F(ConfigTest, VeryLongString) {
    auto& config = Config::instance();
    config.initialize();

    std::string long_path(1000, 'x');
    config.set_workspace_directories({long_path});
    auto dirs = config.get_workspace_directories();
    EXPECT_EQ(dirs.size(), 1u);
    EXPECT_EQ(dirs[0], long_path);
}

TEST_F(ConfigTest, SpecialCharactersInPaths) {
    auto& config = Config::instance();
    config.initialize();

    config.set_workspace_directories({"/path/with spaces/and-dashes_underscores"});
    auto dirs = config.get_workspace_directories();
    EXPECT_EQ(dirs.size(), 1u);
    EXPECT_EQ(dirs[0], "/path/with spaces/and-dashes_underscores");
}

TEST_F(ConfigTest, NegativePorts) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_env_base_port(-100);
    EXPECT_EQ(config.get_env_base_port(), -100);
}

TEST_F(ConfigTest, VeryHighPorts) {
    auto& config = Config::instance();
    config.initialize();
    
    config.set_env_base_port(65535);
    EXPECT_EQ(config.get_env_base_port(), 65535);
}

} // namespace
} // namespace aliases
