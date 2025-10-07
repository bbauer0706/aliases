#include "aliases/config_sync.h"
#include "aliases/config.h"
#include "aliases/file_utils.h"
#include "aliases/process_utils.h"
#include <iostream>
#include <ctime>
#include <sys/stat.h>
#include <fstream>

namespace aliases {

bool ConfigSync::pull() {
    auto& config = Config::instance();

    if (!config.get_sync_enabled()) {
        std::cerr << "Sync is not enabled. Run 'aliases-cli config sync setup <url>' first." << std::endl;
        return false;
    }

    std::string remote_url = config.get_sync_remote_url();
    if (remote_url.empty()) {
        std::cerr << "No remote URL configured." << std::endl;
        return false;
    }

    std::string method = config.get_sync_method();
    std::cout << "Pulling config from " << remote_url << " using " << method << "..." << std::endl;

    bool success = false;
    if (method == "git") {
        success = pull_git();
    } else if (method == "rsync") {
        success = pull_rsync();
    } else if (method == "file") {
        success = pull_file();
    } else if (method == "http") {
        success = pull_http();
    } else {
        std::cerr << "Unknown sync method: " << method << std::endl;
        return false;
    }

    if (success) {
        config.set_sync_last_sync(std::time(nullptr));
        config.save();
        std::cout << Colors::SUCCESS << "✓" << Colors::RESET << " Config synced successfully" << std::endl;

        // Reload config
        config.reload();
    }

    return success;
}

bool ConfigSync::push() {
    auto& config = Config::instance();

    if (!config.get_sync_enabled()) {
        std::cerr << "Sync is not enabled." << std::endl;
        return false;
    }

    std::string remote_url = config.get_sync_remote_url();
    if (remote_url.empty()) {
        std::cerr << "No remote URL configured." << std::endl;
        return false;
    }

    std::string method = config.get_sync_method();
    std::cout << "Pushing config to " << remote_url << " using " << method << "..." << std::endl;

    bool success = false;
    if (method == "git") {
        success = push_git();
    } else if (method == "rsync") {
        success = push_rsync();
    } else if (method == "file") {
        success = push_file();
    } else if (method == "http") {
        std::cerr << "HTTP method does not support push" << std::endl;
        return false;
    } else {
        std::cerr << "Unknown sync method: " << method << std::endl;
        return false;
    }

    if (success) {
        config.set_sync_last_sync(std::time(nullptr));
        config.save();
        std::cout << Colors::SUCCESS << "✓" << Colors::RESET << " Config pushed successfully" << std::endl;
    }

    return success;
}

bool ConfigSync::status() {
    auto& config = Config::instance();

    std::cout << "Sync Configuration:" << std::endl;
    std::cout << "  Enabled: " << (config.get_sync_enabled() ? "yes" : "no") << std::endl;
    std::cout << "  Remote URL: " << config.get_sync_remote_url() << std::endl;
    std::cout << "  Method: " << config.get_sync_method() << std::endl;
    std::cout << "  Auto-sync: " << (config.get_sync_auto_sync() ? "yes" : "no") << std::endl;
    std::cout << "  Sync interval: " << config.get_sync_interval() << " seconds" << std::endl;

    int64_t last_sync = config.get_sync_last_sync();
    if (last_sync > 0) {
        std::time_t sync_time = static_cast<std::time_t>(last_sync);
        std::cout << "  Last sync: " << std::ctime(&sync_time);

        std::time_t now = std::time(nullptr);
        int64_t elapsed = now - last_sync;
        std::cout << "  Time since last sync: " << elapsed << " seconds" << std::endl;
    } else {
        std::cout << "  Last sync: Never" << std::endl;
    }

    return true;
}

bool ConfigSync::setup(const std::string& remote_url, const std::string& method) {
    if (!validate_remote_url(remote_url, method)) {
        std::cerr << "Invalid remote URL or method" << std::endl;
        return false;
    }

    auto& config = Config::instance();
    config.set_sync_remote_url(remote_url);
    config.set_sync_method(method);
    config.set_sync_enabled(true);

    if (!config.save()) {
        std::cerr << "Failed to save configuration" << std::endl;
        return false;
    }

    std::cout << Colors::SUCCESS << "✓" << Colors::RESET
              << " Sync configured with " << method << ": " << remote_url << std::endl;
    std::cout << "Run 'aliases-cli config sync pull' to fetch remote config" << std::endl;

    return true;
}

bool ConfigSync::should_auto_sync() const {
    auto& config = Config::instance();

    if (!config.get_sync_enabled() || !config.get_sync_auto_sync()) {
        return false;
    }

    int64_t last_sync = config.get_sync_last_sync();
    int64_t now = std::time(nullptr);
    int interval = config.get_sync_interval();

    return (now - last_sync) >= interval;
}

bool ConfigSync::auto_sync_if_needed() {
    if (!should_auto_sync()) {
        return true; // Not needed, but not an error
    }

    std::cout << "Auto-syncing configuration..." << std::endl;
    return pull();
}

// ========== Git Implementation ==========

bool ConfigSync::pull_git() {
    auto& config = Config::instance();
    std::string remote_url = config.get_sync_remote_url();
    std::string cache_dir = get_sync_cache_dir();

    if (!ensure_sync_cache_dir()) {
        return false;
    }

    std::string repo_dir = cache_dir + "/config-repo";

    // Clone or pull
    if (!git_clone_or_pull(remote_url, repo_dir)) {
        return false;
    }

    // Copy config files from repo to local config dir
    return copy_config_files(repo_dir, config.get_config_directory());
}

bool ConfigSync::push_git() {
    auto& config = Config::instance();
    std::string remote_url = config.get_sync_remote_url();
    std::string cache_dir = get_sync_cache_dir();
    std::string repo_dir = cache_dir + "/config-repo";

    // Ensure we have the repo
    if (!is_git_repo(repo_dir)) {
        std::cerr << "Git repo not initialized. Run 'config sync pull' first." << std::endl;
        return false;
    }

    // Copy local config to repo
    if (!copy_config_files(config.get_config_directory(), repo_dir)) {
        return false;
    }

    // Commit and push
    return git_commit_and_push(repo_dir, "Update config from " + get_hostname());
}

bool ConfigSync::git_clone_or_pull(const std::string& repo_url, const std::string& target_dir) {
    if (FileUtils::directory_exists(target_dir + "/.git")) {
        // Already cloned, just pull
        std::string command = "cd " + ProcessUtils::escape_shell_argument(target_dir) +
                             " && git pull --quiet";
        return ProcessUtils::execute(command) == 0;
    } else {
        // Clone
        std::string command = "git clone --quiet " +
                             ProcessUtils::escape_shell_argument(repo_url) + " " +
                             ProcessUtils::escape_shell_argument(target_dir);
        return ProcessUtils::execute(command) == 0;
    }
}

bool ConfigSync::git_commit_and_push(const std::string& repo_dir, const std::string& message) {
    std::string commands =
        "cd " + ProcessUtils::escape_shell_argument(repo_dir) + " && " +
        "git add -A && " +
        "git commit -m " + ProcessUtils::escape_shell_argument(message) + " && " +
        "git push --quiet";

    return ProcessUtils::execute(commands) == 0;
}

bool ConfigSync::is_git_repo(const std::string& dir) const {
    return FileUtils::directory_exists(dir + "/.git");
}

// ========== Rsync Implementation ==========

bool ConfigSync::pull_rsync() {
    auto& config = Config::instance();
    std::string remote_url = config.get_sync_remote_url();
    std::string config_dir = config.get_config_directory();

    std::string command = "rsync -az " +
                         ProcessUtils::escape_shell_argument(remote_url + "/") + " " +
                         ProcessUtils::escape_shell_argument(config_dir + "/");

    return ProcessUtils::execute(command) == 0;
}

bool ConfigSync::push_rsync() {
    auto& config = Config::instance();
    std::string remote_url = config.get_sync_remote_url();
    std::string config_dir = config.get_config_directory();

    std::string command = "rsync -az " +
                         ProcessUtils::escape_shell_argument(config_dir + "/") + " " +
                         ProcessUtils::escape_shell_argument(remote_url + "/");

    return ProcessUtils::execute(command) == 0;
}

// ========== File Copy Implementation ==========

bool ConfigSync::pull_file() {
    auto& config = Config::instance();
    std::string remote_url = config.get_sync_remote_url();
    std::string config_dir = config.get_config_directory();

    return copy_config_files(remote_url, config_dir);
}

bool ConfigSync::push_file() {
    auto& config = Config::instance();
    std::string remote_url = config.get_sync_remote_url();
    std::string config_dir = config.get_config_directory();

    return copy_config_files(config_dir, remote_url);
}

bool ConfigSync::copy_config_files(const std::string& source_dir, const std::string& dest_dir) {
    // Copy config.json (but not todos.json - that's local)
    std::vector<std::string> files_to_sync = {"config.json"};

    for (const auto& file : files_to_sync) {
        std::string source_file = source_dir + "/" + file;
        std::string dest_file = dest_dir + "/" + file;

        if (!FileUtils::file_exists(source_file)) {
            continue; // Skip if source doesn't exist
        }

        std::string command = "cp " +
                             ProcessUtils::escape_shell_argument(source_file) + " " +
                             ProcessUtils::escape_shell_argument(dest_file);

        if (ProcessUtils::execute(command) != 0) {
            std::cerr << "Failed to copy " << file << std::endl;
            return false;
        }
    }

    return true;
}

// ========== HTTP Implementation ==========

bool ConfigSync::pull_http() {
    auto& config = Config::instance();
    std::string remote_url = config.get_sync_remote_url();
    std::string config_dir = config.get_config_directory();

    // Download config.json
    std::string command = "curl -s -o " +
                         ProcessUtils::escape_shell_argument(config_dir + "/config.json") + " " +
                         ProcessUtils::escape_shell_argument(remote_url + "/config.json");

    if (ProcessUtils::execute(command) != 0) {
        std::cerr << "Failed to download config.json" << std::endl;
        return false;
    }

    return true;
}

// ========== Helpers ==========

bool ConfigSync::validate_remote_url(const std::string& url, const std::string& method) const {
    if (url.empty()) {
        return false;
    }

    if (method == "git") {
        // Git URL validation (simple check)
        return url.find("git@") == 0 ||
               url.find("https://") == 0 ||
               url.find("http://") == 0 ||
               url.find("file://") == 0;
    } else if (method == "rsync") {
        // Rsync URL validation
        return !url.empty();
    } else if (method == "file") {
        // File path validation
        return url[0] == '/' || url[0] == '~';
    } else if (method == "http") {
        // HTTP URL validation
        return url.find("http://") == 0 || url.find("https://") == 0;
    }

    return false;
}

std::string ConfigSync::get_sync_cache_dir() const {
    return Config::instance().get_cache_directory() + "/sync";
}

bool ConfigSync::ensure_sync_cache_dir() const {
    std::string cache_dir = get_sync_cache_dir();

    struct stat st;
    if (stat(cache_dir.c_str(), &st) == -1) {
        if (mkdir(cache_dir.c_str(), 0755) != 0) {
            std::cerr << "Failed to create sync cache directory" << std::endl;
            return false;
        }
    }

    return true;
}

} // namespace aliases
