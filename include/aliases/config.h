#pragma once

#include "common.h"
#include "third_party/json.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace aliases {

/**
 * Centralized configuration management for aliases CLI
 *
 * Manages all configuration settings from ~/.config/aliases-cli/
 * Uses singleton pattern for global access
 */
class Config {
public:
    // Singleton access
    static Config& instance();

    // Initialize config (call once at startup)
    bool initialize();

    // Reload config from disk
    bool reload();

    // Save current config to disk
    bool save();

    // Reset to default values
    void reset_to_defaults();

    // ========== General Settings ==========

    std::string get_editor() const;
    void set_editor(const std::string& editor);

    bool get_terminal_colors() const;
    void set_terminal_colors(bool enabled);

    std::string get_verbosity() const; // "quiet", "normal", "verbose"
    void set_verbosity(const std::string& level);

    bool get_confirm_destructive_actions() const;
    void set_confirm_destructive_actions(bool confirm);

    // ========== Code Command Settings ==========

    std::vector<std::string> get_vscode_flags() const;
    void set_vscode_flags(const std::vector<std::string>& flags);

    bool get_code_reuse_window() const;
    void set_code_reuse_window(bool reuse);

    std::string get_code_fallback_behavior() const; // "always", "never", "auto"
    void set_code_fallback_behavior(const std::string& behavior);

    std::string get_preferred_component() const; // "server", "web", "ask"
    void set_preferred_component(const std::string& component);

    // ========== Todo Settings ==========

    int get_todo_default_priority() const;
    void set_todo_default_priority(int priority);

    std::string get_todo_default_sort() const; // "priority", "created", "category", "alphabetical"
    void set_todo_default_sort(const std::string& sort);

    bool get_todo_show_completed() const;
    void set_todo_show_completed(bool show);

    bool get_todo_auto_categorize() const;
    void set_todo_auto_categorize(bool enable);

    // ========== Project Environment Settings ==========

    int get_env_base_port() const;
    void set_env_base_port(int port);

    int get_env_port_offset() const;
    void set_env_port_offset(int offset);

    std::string get_env_default_env() const; // "dev", "staging", "prod"
    void set_env_default_env(const std::string& env);

    // ========== Sync Settings ==========

    bool get_sync_enabled() const;
    void set_sync_enabled(bool enabled);

    std::string get_sync_remote_url() const;
    void set_sync_remote_url(const std::string& url);

    bool get_sync_auto_sync() const;
    void set_sync_auto_sync(bool auto_sync);

    int get_sync_interval() const; // Seconds between syncs
    void set_sync_interval(int interval);

    int64_t get_sync_last_sync() const; // Unix timestamp
    void set_sync_last_sync(int64_t timestamp);

    std::string get_sync_method() const; // "git", "rsync", "http", "file"
    void set_sync_method(const std::string& method);

    bool get_sync_todos() const;
    void set_sync_todos(bool sync_todos);

    int64_t get_sync_last_todo_sync() const;
    void set_sync_last_todo_sync(int64_t timestamp);

    // ========== Projects Settings ==========

    std::vector<std::string> get_workspace_directories() const;
    void set_workspace_directories(const std::vector<std::string>& dirs);

    // Get project mappings (returns JSON object references)
    bool get_project_shortcuts(StringMap& shortcuts) const;
    bool get_server_paths(StringMap& paths) const;
    bool get_web_paths(StringMap& paths) const;
    std::vector<std::string> get_default_server_paths() const;
    std::vector<std::string> get_default_web_paths() const;

    std::vector<std::string> get_workspace_ignore() const;
    void set_workspace_ignore(const std::vector<std::string>& ignore_patterns);

    // ========== Path Management ==========

    std::string get_config_directory() const;
    std::string get_config_file_path() const;
    std::string get_todos_file_path() const;
    std::string get_todos_external_file_path() const;
    std::string get_cache_directory() const;

    // ========== Generic Getters/Setters ==========

    // Get any config value as string (for CLI access)
    std::optional<std::string> get(const std::string& key) const;

    // Set any config value from string (for CLI access)
    bool set(const std::string& key, const std::string& value);

    // Get all config as key-value pairs (for listing)
    StringMap get_all() const;

private:
    Config() = default;
    ~Config() = default;

    // Prevent copying
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // Internal data
    std::unique_ptr<nlohmann::json> config_data_;
    bool initialized_ = false;

    // Helper methods
    bool load_from_disk();
    bool ensure_config_directory_exists();
    bool create_default_config();
    void apply_defaults();

    // Default values
    static constexpr const char* DEFAULT_EDITOR = "code";
    static constexpr bool DEFAULT_TERMINAL_COLORS = true;
    static constexpr const char* DEFAULT_VERBOSITY = "normal";
    static constexpr bool DEFAULT_CONFIRM_DESTRUCTIVE = true;

    static constexpr bool DEFAULT_CODE_REUSE_WINDOW = true;
    static constexpr const char* DEFAULT_CODE_FALLBACK = "auto";
    static constexpr const char* DEFAULT_PREFERRED_COMPONENT = "server";

    static constexpr int DEFAULT_TODO_PRIORITY = 0;
    static constexpr const char* DEFAULT_TODO_SORT = "priority";
    static constexpr bool DEFAULT_TODO_SHOW_COMPLETED = false;
    static constexpr bool DEFAULT_TODO_AUTO_CATEGORIZE = false;

    static constexpr int DEFAULT_ENV_BASE_PORT = 3000;
    static constexpr int DEFAULT_ENV_PORT_OFFSET = 100;
    static constexpr const char* DEFAULT_ENV_DEFAULT_ENV = "dev";

    static constexpr bool DEFAULT_SYNC_ENABLED = false;
    static constexpr const char* DEFAULT_SYNC_REMOTE_URL = "";
    static constexpr bool DEFAULT_SYNC_AUTO_SYNC = false;
    static constexpr int DEFAULT_SYNC_INTERVAL = 86400; // 24 hours
    static constexpr int64_t DEFAULT_SYNC_LAST_SYNC = 0;
    static constexpr const char* DEFAULT_SYNC_METHOD = "git";
    static constexpr bool DEFAULT_SYNC_TODOS = false;
    static constexpr int64_t DEFAULT_SYNC_LAST_TODO_SYNC = 0;
};

} // namespace aliases
