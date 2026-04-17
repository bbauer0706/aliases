#pragma once

#include "common.h"
#include "pwd_formatter.h"
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

    // ========== Project Environment Settings ==========

    int get_env_base_port() const;
    void set_env_base_port(int port);

    int get_env_port_offset() const;
    void set_env_port_offset(int offset);

    std::string get_env_default_env() const; // "dev", "staging", "prod"
    void set_env_default_env(const std::string& env);

    // ========== Secrets Settings ==========

    // Full path to the encrypted secrets file.
    // Default: <config_dir>/secrets.enc
    std::string get_secrets_store_path() const;
    void set_secrets_store_path(const std::string& path);

    // PBKDF2 iteration count for master-password key derivation.
    int get_secrets_kdf_iterations() const;
    void set_secrets_kdf_iterations(int iterations);

    // Name of the env var consulted for the master password.
    std::string get_secrets_password_env_var() const;
    void set_secrets_password_env_var(const std::string& var_name);

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

    // ========== Prompt Settings ==========

    bool get_prompt_enabled() const;
    void set_prompt_enabled(bool enabled);

    std::vector<PromptPathReplacement> get_prompt_path_replacements() const;
    void set_prompt_path_replacements(const std::vector<PromptPathReplacement>& replacements);

    // Color for the "user@host" portion of the prompt (e.g. "bold_green").
    // Empty string means no color / inherit terminal default.
    std::string get_prompt_user_host_color() const;
    void set_prompt_user_host_color(const std::string& color);

    // Fallback color for the path when no path_replacement rule matches.
    // Empty string means no color.
    std::string get_prompt_default_path_color() const;
    void set_prompt_default_path_color(const std::string& color);

    // ========== Path Management ==========

    std::string get_config_directory() const;
    std::string get_config_file_path() const;
    std::string get_cache_directory() const;

    // Test mode - allows overriding config directory for isolated testing
    void set_test_config_directory(const std::string& dir);
    void clear_test_config_directory();

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
    std::string test_config_directory_; // When set, overrides real config directory for testing

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


    static constexpr int DEFAULT_ENV_BASE_PORT = 3000;
    static constexpr int DEFAULT_ENV_PORT_OFFSET = 100;
    static constexpr const char* DEFAULT_ENV_DEFAULT_ENV = "dev";

    static constexpr bool DEFAULT_SYNC_ENABLED = false;
    static constexpr const char* DEFAULT_SYNC_REMOTE_URL = "";
    static constexpr bool DEFAULT_SYNC_AUTO_SYNC = false;
    static constexpr int DEFAULT_SYNC_INTERVAL = 86400; // 24 hours
    static constexpr int64_t DEFAULT_SYNC_LAST_SYNC = 0;
    static constexpr const char* DEFAULT_SYNC_METHOD = "git";

    static constexpr bool DEFAULT_PROMPT_ENABLED = true;
    static constexpr const char* DEFAULT_PROMPT_USER_HOST_COLOR = "bold_green";
    static constexpr const char* DEFAULT_PROMPT_DEFAULT_PATH_COLOR = "";

    static constexpr const char* DEFAULT_SECRETS_STORE_PATH   = ""; // empty = use config dir
    static constexpr int         DEFAULT_SECRETS_KDF_ITER      = 100000;
    static constexpr const char* DEFAULT_SECRETS_PASSWORD_VAR = "ALIASES_MASTER_PASSWORD";
};

} // namespace aliases
