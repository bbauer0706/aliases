#include "aliases/config.h"
#include "aliases/file_utils.h"
#include "third_party/json.hpp"
#include <fstream>
#include <sys/stat.h>
#include <thread>

using json = nlohmann::json;

namespace aliases {

Config& Config::instance() {
    static Config instance;
    return instance;
}

bool Config::initialize() {
    if (initialized_) {
        return true;
    }

    // Ensure config directory exists
    if (!ensure_config_directory_exists()) {
        return false;
    }

    // Create JSON object
    config_data_ = std::make_unique<json>();

    // Load config or create defaults
    if (!load_from_disk()) {
        // No config file exists, create default
        if (!create_default_config()) {
            return false;
        }
    }

    // Apply defaults for any missing values
    apply_defaults();

    initialized_ = true;
    return true;
}

bool Config::reload() {
    if (!load_from_disk()) {
        return false;
    }
    apply_defaults();
    return true;
}

bool Config::save() {
    try {
        std::ofstream file(get_config_file_path());
        if (!file.is_open()) {
            return false;
        }

        file << config_data_->dump(2);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void Config::reset_to_defaults() {
    config_data_ = std::make_unique<json>();
    apply_defaults();
}

// ========== Path Management ==========

std::string Config::get_config_directory() const {
    return get_home_directory() + "/.config/aliases-cli";
}

std::string Config::get_config_file_path() const {
    return get_config_directory() + "/config.json";
}

std::string Config::get_todos_file_path() const {
    return get_config_directory() + "/todos.json";
}

std::string Config::get_todos_external_file_path() const {
    return get_config_directory() + "/todos-external.json";
}

std::string Config::get_cache_directory() const {
    return get_config_directory() + "/cache";
}

// ========== General Settings ==========

std::string Config::get_editor() const {
    return (*config_data_)["general"]["editor"].get<std::string>();
}

void Config::set_editor(const std::string& editor) {
    (*config_data_)["general"]["editor"] = editor;
}

bool Config::get_terminal_colors() const {
    return (*config_data_)["general"]["terminal_colors"].get<bool>();
}

void Config::set_terminal_colors(bool enabled) {
    (*config_data_)["general"]["terminal_colors"] = enabled;
}

std::string Config::get_verbosity() const {
    return (*config_data_)["general"]["verbosity"].get<std::string>();
}

void Config::set_verbosity(const std::string& level) {
    (*config_data_)["general"]["verbosity"] = level;
}

bool Config::get_confirm_destructive_actions() const {
    return (*config_data_)["general"]["confirm_destructive_actions"].get<bool>();
}

void Config::set_confirm_destructive_actions(bool confirm) {
    (*config_data_)["general"]["confirm_destructive_actions"] = confirm;
}

// ========== Code Command Settings ==========

std::vector<std::string> Config::get_vscode_flags() const {
    std::vector<std::string> flags;
    if (config_data_->contains("code") && (*config_data_)["code"].contains("vscode_flags")) {
        for (const auto& flag : (*config_data_)["code"]["vscode_flags"]) {
            flags.push_back(flag.get<std::string>());
        }
    }
    return flags;
}

void Config::set_vscode_flags(const std::vector<std::string>& flags) {
    (*config_data_)["code"]["vscode_flags"] = flags;
}

bool Config::get_code_reuse_window() const {
    return (*config_data_)["code"]["reuse_window"].get<bool>();
}

void Config::set_code_reuse_window(bool reuse) {
    (*config_data_)["code"]["reuse_window"] = reuse;
}

std::string Config::get_code_fallback_behavior() const {
    return (*config_data_)["code"]["fallback_behavior"].get<std::string>();
}

void Config::set_code_fallback_behavior(const std::string& behavior) {
    (*config_data_)["code"]["fallback_behavior"] = behavior;
}

std::string Config::get_preferred_component() const {
    return (*config_data_)["code"]["preferred_component"].get<std::string>();
}

void Config::set_preferred_component(const std::string& component) {
    (*config_data_)["code"]["preferred_component"] = component;
}

// ========== Todo Settings ==========

int Config::get_todo_default_priority() const {
    return (*config_data_)["todo"]["default_priority"].get<int>();
}

void Config::set_todo_default_priority(int priority) {
    (*config_data_)["todo"]["default_priority"] = priority;
}

std::string Config::get_todo_default_sort() const {
    return (*config_data_)["todo"]["default_sort"].get<std::string>();
}

void Config::set_todo_default_sort(const std::string& sort) {
    (*config_data_)["todo"]["default_sort"] = sort;
}

bool Config::get_todo_show_completed() const {
    return (*config_data_)["todo"]["show_completed"].get<bool>();
}

void Config::set_todo_show_completed(bool show) {
    (*config_data_)["todo"]["show_completed"] = show;
}

bool Config::get_todo_auto_categorize() const {
    return (*config_data_)["todo"]["auto_categorize"].get<bool>();
}

void Config::set_todo_auto_categorize(bool enable) {
    (*config_data_)["todo"]["auto_categorize"] = enable;
}

// ========== Project Environment Settings ==========

int Config::get_env_base_port() const {
    return (*config_data_)["env"]["base_port"].get<int>();
}

void Config::set_env_base_port(int port) {
    (*config_data_)["env"]["base_port"] = port;
}

int Config::get_env_port_offset() const {
    return (*config_data_)["env"]["port_offset"].get<int>();
}

void Config::set_env_port_offset(int offset) {
    (*config_data_)["env"]["port_offset"] = offset;
}

std::string Config::get_env_default_env() const {
    return (*config_data_)["env"]["default_env"].get<std::string>();
}

void Config::set_env_default_env(const std::string& env) {
    (*config_data_)["env"]["default_env"] = env;
}

// ========== Sync Settings ==========

bool Config::get_sync_enabled() const {
    return (*config_data_)["sync"]["enabled"].get<bool>();
}

void Config::set_sync_enabled(bool enabled) {
    (*config_data_)["sync"]["enabled"] = enabled;
}

bool Config::get_sync_auto_sync_enabled() const {
    if (config_data_->contains("sync") && (*config_data_)["sync"].contains("auto_sync")) {
        auto& auto_sync = (*config_data_)["sync"]["auto_sync"];
        if (auto_sync.is_object() && auto_sync.contains("enabled")) {
            return auto_sync["enabled"].get<bool>();
        }
        // Backward compatibility: if it's a boolean, return it directly
        if (auto_sync.is_boolean()) {
            return auto_sync.get<bool>();
        }
    }
    return DEFAULT_SYNC_AUTO_SYNC_ENABLED;
}

void Config::set_sync_auto_sync_enabled(bool enabled) {
    if (!(*config_data_)["sync"].contains("auto_sync") || !(*config_data_)["sync"]["auto_sync"].is_object()) {
        (*config_data_)["sync"]["auto_sync"] = json::object();
    }
    (*config_data_)["sync"]["auto_sync"]["enabled"] = enabled;
}

int Config::get_sync_auto_sync_interval() const {
    if (config_data_->contains("sync") && (*config_data_)["sync"].contains("auto_sync")) {
        auto& auto_sync = (*config_data_)["sync"]["auto_sync"];
        if (auto_sync.is_object() && auto_sync.contains("interval")) {
            return auto_sync["interval"].get<int>();
        }
    }
    // Backward compatibility: check for old sync_interval field
    if (config_data_->contains("sync") && (*config_data_)["sync"].contains("sync_interval")) {
        return (*config_data_)["sync"]["sync_interval"].get<int>();
    }
    return DEFAULT_SYNC_AUTO_SYNC_INTERVAL;
}

void Config::set_sync_auto_sync_interval(int interval) {
    if (!(*config_data_)["sync"].contains("auto_sync") || !(*config_data_)["sync"]["auto_sync"].is_object()) {
        (*config_data_)["sync"]["auto_sync"] = json::object();
    }
    (*config_data_)["sync"]["auto_sync"]["interval"] = interval;
}

int64_t Config::get_sync_last_sync() const {
    return (*config_data_)["sync"]["last_sync"].get<int64_t>();
}

void Config::set_sync_last_sync(int64_t timestamp) {
    (*config_data_)["sync"]["last_sync"] = timestamp;
}

std::string Config::get_sync_config_file_url() const {
    if (config_data_->contains("sync") && (*config_data_)["sync"].contains("config_file_url")) {
        return (*config_data_)["sync"]["config_file_url"].get<std::string>();
    }
    // Backward compatibility: fall back to old remote_url
    if (config_data_->contains("sync") && (*config_data_)["sync"].contains("remote_url")) {
        return (*config_data_)["sync"]["remote_url"].get<std::string>();
    }
    return DEFAULT_SYNC_CONFIG_FILE_URL;
}

void Config::set_sync_config_file_url(const std::string& url) {
    (*config_data_)["sync"]["config_file_url"] = url;
}

std::string Config::get_sync_todo_file_url() const {
    if (config_data_->contains("sync") && (*config_data_)["sync"].contains("todo_file_url")) {
        return (*config_data_)["sync"]["todo_file_url"].get<std::string>();
    }
    return DEFAULT_SYNC_TODO_FILE_URL;
}

void Config::set_sync_todo_file_url(const std::string& url) {
    (*config_data_)["sync"]["todo_file_url"] = url;
}

// ========== Projects Settings ==========

std::vector<std::string> Config::get_workspace_directories() const {
    std::vector<std::string> directories;

    if (config_data_->contains("projects")) {
        // Check for new array format
        if ((*config_data_)["projects"].contains("workspace_directories") &&
            (*config_data_)["projects"]["workspace_directories"].is_array()) {
            for (const auto& dir : (*config_data_)["projects"]["workspace_directories"]) {
                directories.push_back(dir.get<std::string>());
            }
        }
        // Fallback to old singular format for backwards compatibility
        else if ((*config_data_)["projects"].contains("workspace_directory")) {
            directories.push_back((*config_data_)["projects"]["workspace_directory"].get<std::string>());
        }
    }

    // Default if nothing configured
    if (directories.empty()) {
        directories.push_back("~/workspaces");
    }

    return directories;
}

void Config::set_workspace_directories(const std::vector<std::string>& dirs) {
    if (!config_data_->contains("projects")) {
        (*config_data_)["projects"] = json::object();
    }
    (*config_data_)["projects"]["workspace_directories"] = dirs;
}

bool Config::get_project_shortcuts(StringMap& shortcuts) const {
    shortcuts.clear();
    if (config_data_->contains("projects") && (*config_data_)["projects"].contains("shortcuts")) {
        for (const auto& [key, value] : (*config_data_)["projects"]["shortcuts"].items()) {
            shortcuts[key] = value.get<std::string>();
        }
        return true;
    }
    return false;
}

bool Config::get_server_paths(StringMap& paths) const {
    paths.clear();
    if (config_data_->contains("projects") && (*config_data_)["projects"].contains("server_paths")) {
        for (const auto& [key, value] : (*config_data_)["projects"]["server_paths"].items()) {
            paths[key] = value.get<std::string>();
        }
        return true;
    }
    return false;
}

bool Config::get_web_paths(StringMap& paths) const {
    paths.clear();
    if (config_data_->contains("projects") && (*config_data_)["projects"].contains("web_paths")) {
        for (const auto& [key, value] : (*config_data_)["projects"]["web_paths"].items()) {
            paths[key] = value.get<std::string>();
        }
        return true;
    }
    return false;
}

std::vector<std::string> Config::get_default_server_paths() const {
    std::vector<std::string> paths;
    if (config_data_->contains("projects") &&
        (*config_data_)["projects"].contains("default_paths") &&
        (*config_data_)["projects"]["default_paths"].contains("server")) {
        for (const auto& path : (*config_data_)["projects"]["default_paths"]["server"]) {
            paths.push_back(path.get<std::string>());
        }
    }
    if (paths.empty()) {
        // Return hardcoded defaults if not in config
        paths = {"java/serverJava", "serverJava", "backend", "server"};
    }
    return paths;
}

std::vector<std::string> Config::get_default_web_paths() const {
    std::vector<std::string> paths;
    if (config_data_->contains("projects") &&
        (*config_data_)["projects"].contains("default_paths") &&
        (*config_data_)["projects"]["default_paths"].contains("web")) {
        for (const auto& path : (*config_data_)["projects"]["default_paths"]["web"]) {
            paths.push_back(path.get<std::string>());
        }
    }
    if (paths.empty()) {
        // Return hardcoded defaults if not in config
        paths = {"webapp", "webApp", "web", "frontend", "client"};
    }
    return paths;
}

std::vector<std::string> Config::get_workspace_ignore() const {
    std::vector<std::string> ignore_patterns;
    if (config_data_->contains("projects") && (*config_data_)["projects"].contains("ignore")) {
        for (const auto& pattern : (*config_data_)["projects"]["ignore"]) {
            ignore_patterns.push_back(pattern.get<std::string>());
        }
    }
    return ignore_patterns;
}

void Config::set_workspace_ignore(const std::vector<std::string>& ignore_patterns) {
    if (!config_data_->contains("projects")) {
        (*config_data_)["projects"] = json::object();
    }
    (*config_data_)["projects"]["ignore"] = ignore_patterns;
}

// ========== Generic Getters/Setters ==========

std::optional<std::string> Config::get(const std::string& key) const {
    // Parse dot-separated key path
    std::vector<std::string> path;
    std::string current;
    for (char c : key) {
        if (c == '.') {
            if (!current.empty()) {
                path.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        path.push_back(current);
    }

    // Navigate JSON structure
    try {
        json current_node = *config_data_;
        for (const auto& segment : path) {
            if (!current_node.contains(segment)) {
                return std::nullopt;
            }
            current_node = current_node[segment];
        }

        // Convert to string
        if (current_node.is_string()) {
            return current_node.get<std::string>();
        } else if (current_node.is_boolean()) {
            return current_node.get<bool>() ? "true" : "false";
        } else if (current_node.is_number()) {
            return std::to_string(current_node.get<int>());
        }

        return std::nullopt;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

bool Config::set(const std::string& key, const std::string& value) {
    // Parse dot-separated key path
    std::vector<std::string> path;
    std::string current;
    for (char c : key) {
        if (c == '.') {
            if (!current.empty()) {
                path.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        path.push_back(current);
    }

    if (path.empty()) {
        return false;
    }

    try {
        // Navigate to parent node
        json* current_node = config_data_.get();
        for (size_t i = 0; i < path.size() - 1; ++i) {
            if (!current_node->contains(path[i])) {
                (*current_node)[path[i]] = json::object();
            }
            current_node = &(*current_node)[path[i]];
        }

        const std::string& last_key = path.back();

        // Try to preserve type
        if (current_node->contains(last_key)) {
            if ((*current_node)[last_key].is_boolean()) {
                (*current_node)[last_key] = (value == "true" || value == "1");
            } else if ((*current_node)[last_key].is_number_integer()) {
                (*current_node)[last_key] = std::stoi(value);
            } else {
                (*current_node)[last_key] = value;
            }
        } else {
            // New key, set as string
            (*current_node)[last_key] = value;
        }

        return true;
    } catch (const std::exception&) {
        return false;
    }
}

StringMap Config::get_all() const {
    StringMap result;

    // Flatten JSON structure
    std::function<void(const json&, const std::string&)> flatten;
    flatten = [&](const json& node, const std::string& prefix) {
        if (node.is_object()) {
            for (auto it = node.begin(); it != node.end(); ++it) {
                std::string key = prefix.empty() ? it.key() : prefix + "." + it.key();
                flatten(it.value(), key);
            }
        } else if (node.is_string()) {
            result[prefix] = node.get<std::string>();
        } else if (node.is_boolean()) {
            result[prefix] = node.get<bool>() ? "true" : "false";
        } else if (node.is_number()) {
            result[prefix] = std::to_string(node.get<int>());
        } else if (node.is_array()) {
            result[prefix] = node.dump();
        }
    };

    flatten(*config_data_, "");
    return result;
}

// ========== Helper Methods ==========

bool Config::load_from_disk() {
    try {
        std::ifstream file(get_config_file_path());
        if (!file.is_open()) {
            return false;
        }

        *config_data_ = json::parse(file);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool Config::ensure_config_directory_exists() {
    std::string config_dir = get_config_directory();
    std::string cache_dir = get_cache_directory();

    struct stat st;
    if (stat(config_dir.c_str(), &st) == -1) {
        if (mkdir(config_dir.c_str(), 0755) != 0) {
            return false;
        }
    }

    if (stat(cache_dir.c_str(), &st) == -1) {
        mkdir(cache_dir.c_str(), 0755); // OK if this fails
    }

    return true;
}

bool Config::create_default_config() {
    apply_defaults();
    return save();
}

void Config::apply_defaults() {
    json& cfg = *config_data_;

    // General
    if (!cfg.contains("general")) cfg["general"] = json::object();
    if (!cfg["general"].contains("editor")) cfg["general"]["editor"] = DEFAULT_EDITOR;
    if (!cfg["general"].contains("terminal_colors")) cfg["general"]["terminal_colors"] = DEFAULT_TERMINAL_COLORS;
    if (!cfg["general"].contains("verbosity")) cfg["general"]["verbosity"] = DEFAULT_VERBOSITY;
    if (!cfg["general"].contains("confirm_destructive_actions"))
        cfg["general"]["confirm_destructive_actions"] = DEFAULT_CONFIRM_DESTRUCTIVE;

    // Code
    if (!cfg.contains("code")) cfg["code"] = json::object();
    if (!cfg["code"].contains("vscode_flags")) cfg["code"]["vscode_flags"] = json::array();
    if (!cfg["code"].contains("reuse_window")) cfg["code"]["reuse_window"] = DEFAULT_CODE_REUSE_WINDOW;
    if (!cfg["code"].contains("fallback_behavior")) cfg["code"]["fallback_behavior"] = DEFAULT_CODE_FALLBACK;
    if (!cfg["code"].contains("preferred_component")) cfg["code"]["preferred_component"] = DEFAULT_PREFERRED_COMPONENT;

    // Todo
    if (!cfg.contains("todo")) cfg["todo"] = json::object();
    if (!cfg["todo"].contains("default_priority")) cfg["todo"]["default_priority"] = DEFAULT_TODO_PRIORITY;
    if (!cfg["todo"].contains("default_sort")) cfg["todo"]["default_sort"] = DEFAULT_TODO_SORT;
    if (!cfg["todo"].contains("show_completed")) cfg["todo"]["show_completed"] = DEFAULT_TODO_SHOW_COMPLETED;
    if (!cfg["todo"].contains("auto_categorize")) cfg["todo"]["auto_categorize"] = DEFAULT_TODO_AUTO_CATEGORIZE;

    // Env
    if (!cfg.contains("env")) cfg["env"] = json::object();
    if (!cfg["env"].contains("base_port")) cfg["env"]["base_port"] = DEFAULT_ENV_BASE_PORT;
    if (!cfg["env"].contains("port_offset")) cfg["env"]["port_offset"] = DEFAULT_ENV_PORT_OFFSET;
    if (!cfg["env"].contains("default_env")) cfg["env"]["default_env"] = DEFAULT_ENV_DEFAULT_ENV;

    // Sync - new structure with migration from old format
    if (!cfg.contains("sync")) cfg["sync"] = json::object();
    if (!cfg["sync"].contains("enabled")) cfg["sync"]["enabled"] = DEFAULT_SYNC_ENABLED;
    if (!cfg["sync"].contains("last_sync")) cfg["sync"]["last_sync"] = DEFAULT_SYNC_LAST_SYNC;
    if (!cfg["sync"].contains("config_file_url")) cfg["sync"]["config_file_url"] = DEFAULT_SYNC_CONFIG_FILE_URL;
    if (!cfg["sync"].contains("todo_file_url")) cfg["sync"]["todo_file_url"] = DEFAULT_SYNC_TODO_FILE_URL;

    // Handle auto_sync: migrate old boolean format to new object format
    if (!cfg["sync"].contains("auto_sync")) {
        cfg["sync"]["auto_sync"] = json::object();
        cfg["sync"]["auto_sync"]["enabled"] = DEFAULT_SYNC_AUTO_SYNC_ENABLED;
        cfg["sync"]["auto_sync"]["interval"] = DEFAULT_SYNC_AUTO_SYNC_INTERVAL;
    } else if (cfg["sync"]["auto_sync"].is_boolean()) {
        // Migrate old boolean to new object format
        bool old_value = cfg["sync"]["auto_sync"].get<bool>();
        cfg["sync"]["auto_sync"] = json::object();
        cfg["sync"]["auto_sync"]["enabled"] = old_value;
        cfg["sync"]["auto_sync"]["interval"] = DEFAULT_SYNC_AUTO_SYNC_INTERVAL;
    } else if (cfg["sync"]["auto_sync"].is_object()) {
        // Ensure both fields exist
        if (!cfg["sync"]["auto_sync"].contains("enabled")) {
            cfg["sync"]["auto_sync"]["enabled"] = DEFAULT_SYNC_AUTO_SYNC_ENABLED;
        }
        if (!cfg["sync"]["auto_sync"].contains("interval")) {
            cfg["sync"]["auto_sync"]["interval"] = DEFAULT_SYNC_AUTO_SYNC_INTERVAL;
        }
    }

    // Clean up old sync fields if they exist (migration)
    if (cfg["sync"].contains("remote_url")) cfg["sync"].erase("remote_url");
    if (cfg["sync"].contains("method")) cfg["sync"].erase("method");
    if (cfg["sync"].contains("sync_todos")) cfg["sync"].erase("sync_todos");
    if (cfg["sync"].contains("last_todo_sync")) cfg["sync"].erase("last_todo_sync");
    if (cfg["sync"].contains("sync_interval")) cfg["sync"].erase("sync_interval");

    // Projects
    if (!cfg.contains("projects")) cfg["projects"] = json::object();

    // Handle migration from old workspace_directory to new workspace_directories
    if (cfg["projects"].contains("workspace_directory") && !cfg["projects"].contains("workspace_directories")) {
        // Migrate old singular format to new array format
        std::string old_dir = cfg["projects"]["workspace_directory"].get<std::string>();
        cfg["projects"]["workspace_directories"] = json::array({old_dir});
        cfg["projects"].erase("workspace_directory");
    }

    if (!cfg["projects"].contains("workspace_directories")) {
        cfg["projects"]["workspace_directories"] = json::array({"~/workspaces"});
    }
    if (!cfg["projects"].contains("shortcuts")) cfg["projects"]["shortcuts"] = json::object();
    if (!cfg["projects"].contains("server_paths")) cfg["projects"]["server_paths"] = json::object();
    if (!cfg["projects"].contains("web_paths")) cfg["projects"]["web_paths"] = json::object();
    if (!cfg["projects"].contains("ignore")) cfg["projects"]["ignore"] = json::array();
    if (!cfg["projects"].contains("default_paths")) {
        cfg["projects"]["default_paths"] = json::object();
        cfg["projects"]["default_paths"]["server"] = json::array({"java/serverJava", "serverJava", "backend", "server"});
        cfg["projects"]["default_paths"]["web"] = json::array({"webapp", "webApp", "web", "frontend", "client"});
    }
}

} // namespace aliases
