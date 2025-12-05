#include "aliases/config_sync.h"
#include "aliases/config.h"
#include "aliases/file_utils.h"
#include "aliases/process_utils.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

namespace aliases {

bool ConfigSync::fetch_file(const std::string& url, const std::string& destination) {
    std::string command = "curl -f -s -o " + ProcessUtils::escape_shell_argument(destination) + " " +
                          ProcessUtils::escape_shell_argument(url);

    return ProcessUtils::execute(command).success();
}

bool ConfigSync::pull_config_file() {
    auto& config = Config::instance();

    if (!config.get_sync_enabled()) {
        std::cerr << "Sync is not enabled. Run 'aliases-cli config sync setup <url>' first." << std::endl;
        return false;
    }

    std::string config_url = config.get_sync_config_file_url();
    if (config_url.empty()) {
        std::cerr << "Config file URL not configured." << std::endl;
        return false;
    }

    std::string config_path = config.get_config_file_path();
    if (!fetch_file(config_url, config_path)) {
        std::cerr << "Failed to download config.json" << std::endl;
        return false;
    }

    return true;
}

bool ConfigSync::pull() {
    auto& config = Config::instance();

    if (!config.get_sync_enabled()) {
        std::cerr << "Sync is not enabled. Run 'aliases-cli config sync setup <url>' first." << std::endl;
        return false;
    }

    std::string config_url = config.get_sync_config_file_url();

    if (config_url.empty()) {
        std::cerr << "No config file URL configured for sync." << std::endl;
        return false;
    }

    std::cout << "Fetching config file..." << std::endl;

    bool success = true;

    // Fetch config.json
    std::cout << "  Downloading config from " << config_url << "..." << std::endl;

    if (!pull_config_file()) {
        std::cerr << "  Failed to download config.json" << std::endl;
        success = false;
    } else {
        std::cout << "  " << Colors::SUCCESS << "✓" << Colors::RESET << " Config downloaded" << std::endl;
    }

    if (success) {
        config.set_sync_last_sync(std::time(nullptr));
        config.save();
        std::cout << Colors::SUCCESS << "✓" << Colors::RESET << " Sync completed successfully" << std::endl;

        // Reload config
        config.reload();
    }

    return success;
}

bool ConfigSync::push() {
    std::cerr << "Push is not supported with the current sync implementation." << std::endl;
    std::cerr << "The sync feature now uses simple HTTP fetch for read-only config distribution." << std::endl;
    std::cerr << "To share your config, upload it to a web server or Git repository and share the URL." << std::endl;
    return false;
}

bool ConfigSync::status() {
    auto& config = Config::instance();

    std::cout << "Sync Configuration:" << std::endl;
    std::cout << "  Enabled: " << (config.get_sync_enabled() ? "yes" : "no") << std::endl;
    std::cout << "  Config file URL: "
              << (config.get_sync_config_file_url().empty() ? "(not set)" : config.get_sync_config_file_url())
              << std::endl;
    std::cout << "  Auto-sync enabled: " << (config.get_sync_auto_sync_enabled() ? "yes" : "no") << std::endl;
    std::cout << "  Auto-sync interval: " << config.get_sync_auto_sync_interval() << " seconds" << std::endl;

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

bool ConfigSync::setup(const std::string& config_url) {
    auto& config = Config::instance();

    // Validate URL
    if (config_url.empty() || config_url == "-") {
        std::cerr << "Config URL must be provided" << std::endl;
        return false;
    }

    // Set URL
    config.set_sync_config_file_url(config_url);
    config.set_sync_enabled(true);

    if (!config.save()) {
        std::cerr << "Failed to save configuration" << std::endl;
        return false;
    }

    std::cout << Colors::SUCCESS << "✓" << Colors::RESET << " Sync configured" << std::endl;
    std::cout << "  Config URL: " << config_url << std::endl;
    std::cout << "Run 'aliases-cli config sync pull' to fetch remote config" << std::endl;

    return true;
}

bool ConfigSync::should_auto_sync() const {
    auto& config = Config::instance();

    if (!config.get_sync_enabled() || !config.get_sync_auto_sync_enabled()) {
        return false;
    }

    int64_t last_sync = config.get_sync_last_sync();
    int64_t now = std::time(nullptr);
    int interval = config.get_sync_auto_sync_interval();

    return (now - last_sync) >= interval;
}

bool ConfigSync::auto_sync_if_needed() {
    if (!should_auto_sync()) {
        return true; // Not needed, but not an error
    }

    std::cout << "Auto-syncing configuration..." << std::endl;
    return pull();
}

} // namespace aliases
