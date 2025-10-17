#include "aliases/commands/config_cmd.h"
#include "aliases/config.h"
#include "aliases/config_sync.h"
#include "aliases/process_utils.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

namespace aliases::commands {

ConfigCmd::ConfigCmd(std::shared_ptr<ProjectMapper> mapper) : project_mapper_(std::move(mapper)) {}

int ConfigCmd::execute(const StringVector& args) {
    // Handle help
    if (args.empty() || args[0] == "-h" || args[0] == "--help") {
        show_help();
        return 0;
    }

    const std::string& subcommand = args[0];

    if (subcommand == "get") {
        return cmd_get(args);
    } else if (subcommand == "set") {
        return cmd_set(args);
    } else if (subcommand == "list" || subcommand == "ls") {
        return cmd_list(args);
    } else if (subcommand == "reset") {
        return cmd_reset(args);
    } else if (subcommand == "edit") {
        return cmd_edit(args);
    } else if (subcommand == "path") {
        return cmd_path(args);
    } else if (subcommand == "sync") {
        // Handle sync subcommands
        if (args.size() < 2) {
            std::cerr << "Usage: aliases-cli config sync <setup|pull|status|push>" << std::endl;
            return 1;
        }

        const std::string& sync_cmd = args[1];
        ConfigSync sync_manager;

        if (sync_cmd == "pull") {
            return sync_manager.pull() ? 0 : 1;
        } else if (sync_cmd == "push") {
            return sync_manager.push() ? 0 : 1;
        } else if (sync_cmd == "status") {
            return sync_manager.status() ? 0 : 1;
        } else if (sync_cmd == "setup") {
            if (args.size() < 3) {
                std::cerr << "Usage: aliases-cli config sync setup <config-url> [todo-url]" << std::endl;
                std::cerr << "  config-url: URL to config.json file (or '-' to skip)" << std::endl;
                std::cerr << "  todo-url:   URL to todos.json file (optional, or '-' to skip)" << std::endl;
                return 1;
            }
            std::string config_url = args[2];
            std::string todo_url = args.size() > 3 ? args[3] : "";
            return sync_manager.setup(config_url, todo_url) ? 0 : 1;
        } else {
            std::cerr << "Unknown sync subcommand: " << sync_cmd << std::endl;
            std::cerr << "Available: pull, push, status, setup" << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Unknown subcommand: " << subcommand << std::endl;
        show_help();
        return 1;
    }
}

void ConfigCmd::show_help() const {
    std::cout << "Config management for aliases-cli" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: aliases-cli config <subcommand> [args...]" << std::endl;
    std::cout << std::endl;
    std::cout << "Subcommands:" << std::endl;
    std::cout << "  get <key>           Get configuration value" << std::endl;
    std::cout << "  set <key> <value>   Set configuration value" << std::endl;
    std::cout << "  list, ls            List all configuration settings" << std::endl;
    std::cout << "  reset               Reset configuration to defaults" << std::endl;
    std::cout << "  edit                Open config file in editor" << std::endl;
    std::cout << "  path                Show config file path" << std::endl;
    std::cout << std::endl;
    std::cout << "Sync subcommands:" << std::endl;
    std::cout << "  sync setup <config-url> [todo-url]  Setup config sync with file-specific URLs" << std::endl;
    std::cout << "  sync pull                            Pull config from remote URLs" << std::endl;
    std::cout << "  sync push                            Push config to remote (not supported)" << std::endl;
    std::cout << "  sync status                          Show sync status" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  aliases-cli config get general.editor" << std::endl;
    std::cout << "  aliases-cli config set general.editor vim" << std::endl;
    std::cout << "  aliases-cli config set code.reuse_window false" << std::endl;
    std::cout << "  aliases-cli config list" << std::endl;
    std::cout << "  aliases-cli config edit" << std::endl;
    std::cout << std::endl;
    std::cout << "  # Setup sync with direct file URLs" << std::endl;
    std::cout << "  aliases-cli config sync setup https://example.com/config.json https://example.com/todos.json" << std::endl;
    std::cout << "  aliases-cli config sync setup https://example.com/config.json  # Config only" << std::endl;
    std::cout << "  aliases-cli config sync pull" << std::endl;
    std::cout << std::endl;
    std::cout << "Configuration categories:" << std::endl;
    std::cout << "  general.*      - General settings (editor, colors, verbosity)" << std::endl;
    std::cout << "  code.*         - Code command settings" << std::endl;
    std::cout << "  todo.*         - Todo command settings" << std::endl;
    std::cout << "  env.*          - Environment command settings" << std::endl;
    std::cout << "  sync.*         - Config sync settings" << std::endl;
}

int ConfigCmd::cmd_get(const StringVector& args) {
    if (args.size() < 2) {
        std::cerr << "Usage: aliases-cli config get <key>" << std::endl;
        return 1;
    }

    const std::string& key = args[1];
    auto& config = Config::instance();

    auto value = config.get(key);
    if (value) {
        std::cout << *value << std::endl;
        return 0;
    } else {
        std::cerr << Colors::ERROR << "✗" << Colors::RESET << " Config key '" << key << "' not found" << std::endl;
        return 1;
    }
}

int ConfigCmd::cmd_set(const StringVector& args) {
    if (args.size() < 3) {
        std::cerr << "Usage: aliases-cli config set <key> <value>" << std::endl;
        return 1;
    }

    const std::string& key = args[1];
    const std::string& value = args[2];
    auto& config = Config::instance();

    if (config.set(key, value)) {
        if (config.save()) {
            std::cout << Colors::SUCCESS << "✓" << Colors::RESET << " Set " << key << " = " << value << std::endl;
            return 0;
        } else {
            std::cerr << Colors::ERROR << "✗" << Colors::RESET << " Failed to save configuration" << std::endl;
            return 1;
        }
    } else {
        std::cerr << Colors::ERROR << "✗" << Colors::RESET << " Failed to set config key '" << key << "'" << std::endl;
        return 1;
    }
}

int ConfigCmd::cmd_list(const StringVector& /* args */) {
    auto& config = Config::instance();
    auto all_config = config.get_all();

    if (all_config.empty()) {
        std::cout << "No configuration found." << std::endl;
        return 0;
    }

    std::cout << "Current configuration:" << std::endl;
    std::cout << std::endl;

    // Group by category
    std::vector<std::pair<std::string, std::string>> sorted_config(all_config.begin(), all_config.end());
    std::sort(sorted_config.begin(), sorted_config.end());

    std::string current_category;
    for (const auto& [key, value] : sorted_config) {
        // Extract category (first part before dot)
        size_t dot_pos = key.find('.');
        std::string category = dot_pos != std::string::npos ? key.substr(0, dot_pos) : "";

        if (category != current_category) {
            if (!current_category.empty()) {
                std::cout << std::endl;
            }
            std::cout << Colors::INFO << "[" << category << "]" << Colors::RESET << std::endl;
            current_category = category;
        }

        // Print key-value pair with indentation
        std::cout << "  " << std::setw(30) << std::left << key << " = " << value << std::endl;
    }

    return 0;
}

int ConfigCmd::cmd_reset(const StringVector& /* args */) {
    auto& config = Config::instance();

    std::cout << Colors::WARNING << "⚠" << Colors::RESET
              << " This will reset all configuration to defaults. Continue? (y/N): ";

    char response;
    std::cin >> response;

    if (response != 'y' && response != 'Y') {
        std::cout << "Cancelled." << std::endl;
        return 0;
    }

    config.reset_to_defaults();
    if (config.save()) {
        std::cout << Colors::SUCCESS << "✓" << Colors::RESET << " Configuration reset to defaults" << std::endl;
        return 0;
    } else {
        std::cerr << Colors::ERROR << "✗" << Colors::RESET << " Failed to save configuration" << std::endl;
        return 1;
    }
}

int ConfigCmd::cmd_edit(const StringVector& /* args */) {
    auto& config = Config::instance();
    std::string editor = config.get_editor();
    std::string config_path = config.get_config_file_path();

    std::string command = editor + " " + ProcessUtils::escape_shell_argument(config_path);

    std::cout << "Opening config in " << editor << "..." << std::endl;
    ProcessUtils::execute(command);

    // Reload config after editing
    if (config.reload()) {
        std::cout << Colors::SUCCESS << "✓" << Colors::RESET << " Configuration reloaded" << std::endl;
        return 0;
    } else {
        std::cerr << Colors::WARNING << "⚠" << Colors::RESET << " Warning: Failed to reload configuration" << std::endl;
        return 1;
    }
}

int ConfigCmd::cmd_path(const StringVector& /* args */) {
    auto& config = Config::instance();
    std::cout << config.get_config_file_path() << std::endl;
    return 0;
}

} // namespace aliases::commands
