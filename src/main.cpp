#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "aliases/commands/code_navigator.h"
#include "aliases/commands/config_cmd.h"
#include "aliases/commands/project_env.h"
#include "aliases/commands/todo.h"
#include "aliases/config.h"
#include "aliases/config_sync.h"
#include "aliases/project_mapper.h"

namespace {
#ifndef VERSION
#define VERSION "dev"
#endif
constexpr const char* PROGRAM_NAME = "aliases-cli";
} // namespace

void show_version() {
    std::cout << PROGRAM_NAME << " version " << VERSION << std::endl;
    std::cout << "A fast C++ workspace management tool" << std::endl;
}

void show_help() {
    std::cout << "Usage: " << PROGRAM_NAME << " <command> [arguments...]" << std::endl;
    std::cout << std::endl;
    std::cout << "A fast workspace management CLI tool" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  code, c          VS Code project navigation" << std::endl;
    std::cout << "  env              Setup project environment variables" << std::endl;
    std::cout << "  todo             Todo list manager with CLI and TUI modes" << std::endl;
    std::cout << "  config           Manage aliases-cli configuration" << std::endl;
    std::cout << "  completion       Generate completion data (for bash completion)" << std::endl;
    std::cout << "  version          Show version information" << std::endl;
    std::cout << "  help             Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Use '" << PROGRAM_NAME << " <command> --help' for more information on a command." << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << PROGRAM_NAME << " code urm          # Open project 'urm' in VS Code" << std::endl;
    std::cout << "  " << PROGRAM_NAME << " env -p 3000       # Setup environment with port 3000" << std::endl;
    std::cout << "  " << PROGRAM_NAME << " todo              # Launch interactive todo TUI" << std::endl;
    std::cout << "  " << PROGRAM_NAME << " todo add \"Fix bug\" # Add a new todo via CLI" << std::endl;
    std::cout << "  " << PROGRAM_NAME << " config list       # View all configuration" << std::endl;
}

int handle_completion(std::shared_ptr<aliases::ProjectMapper> project_mapper, const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: completion command requires a subcommand" << std::endl;
        std::cerr << "Usage: " << PROGRAM_NAME << " completion <subcommand>" << std::endl;
        std::cerr << "Subcommands: projects, components, todo" << std::endl;
        return 1;
    }

    const std::string& subcommand = args[0];

    if (subcommand == "projects") {
        // Output all projects in a simple format: one per line, shortcut|fullname|has_server|has_web
        auto projects = project_mapper->get_all_projects();
        for (const auto& project : projects) {
            std::cout << project.display_name << "|" << project.full_name << "|"
                      << (project.has_server_component ? "s" : "-") << "|" << (project.has_web_component ? "w" : "-")
                      << std::endl;
        }
        return 0;
    } else if (subcommand == "components" && args.size() >= 2) {
        // Output components for a specific project: project|component_type|path
        const std::string& project_name = args[1];
        auto project_info = project_mapper->get_project_info(project_name);

        if (!project_info) {
            return 1; // Project not found, silent fail for completion
        }

        // Output server component if available
        if (project_info->has_server_component) {
            std::cout << project_info->display_name << "|s|" << project_info->server_path.value_or("") << std::endl;
        }

        // Output web component if available
        if (project_info->has_web_component) {
            std::cout << project_info->display_name << "|w|" << project_info->web_path.value_or("") << std::endl;
        }

        return 0;
    } else if (subcommand == "todo") {
        // Output todo command completions
        std::cout << "add" << std::endl;
        std::cout << "list" << std::endl;
        std::cout << "ls" << std::endl;
        std::cout << "done" << std::endl;
        std::cout << "complete" << std::endl;
        std::cout << "remove" << std::endl;
        std::cout << "rm" << std::endl;
        std::cout << "delete" << std::endl;
        std::cout << "priority" << std::endl;
        std::cout << "prio" << std::endl;
        std::cout << "category" << std::endl;
        std::cout << "cat" << std::endl;
        std::cout << "tui" << std::endl;
        std::cout << "-i" << std::endl;
        std::cout << "--interactive" << std::endl;
        std::cout << "--help" << std::endl;
        std::cout << "-h" << std::endl;
        return 0;
    } else {
        std::cerr << "Error: Unknown completion subcommand '" << subcommand << "'" << std::endl;
        return 1;
    }
}

int main(int argc, char* argv[]) {
    // Convert C-style args to vector
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    // Handle no arguments
    if (args.empty()) {
        show_help();
        return 0;
    }

    // Handle global flags
    const std::string& command = args[0];
    if (command == "--version" || command == "-v") {
        show_version();
        return 0;
    }

    if (command == "--help" || command == "-h" || command == "help") {
        show_help();
        return 0;
    }

    // Initialize configuration (must be done first)
    auto& config = aliases::Config::instance();
    if (!config.initialize()) {
        std::cerr << "Warning: Failed to initialize configuration, using defaults" << std::endl;
    }

    // Auto-sync if enabled and needed (only for config and todo commands)
    if (command == "config" || command == "todo") {
        aliases::ConfigSync sync_manager;
        sync_manager.auto_sync_if_needed();
    }

    // Initialize project mapper (shared across all commands)
    auto project_mapper = std::make_shared<aliases::ProjectMapper>();
    if (!project_mapper->initialize()) {
        std::cerr << "Error: Failed to initialize project mapper" << std::endl;
        return 1;
    }

    // Remove command from args for passing to subcommands
    std::vector<std::string> subcommand_args(args.begin() + 1, args.end());

    // Dispatch to appropriate command
    try {
        if (command == "code" || command == "c") {
            aliases::commands::CodeNavigator navigator(project_mapper);
            return navigator.execute(subcommand_args);
        } else if (command == "env") {
            aliases::commands::ProjectEnv env_setup(project_mapper);
            return env_setup.execute(subcommand_args);
        } else if (command == "todo") {
            aliases::commands::Todo todo_cmd(project_mapper);
            return todo_cmd.execute(subcommand_args);
        } else if (command == "config") {
            aliases::commands::ConfigCmd config_cmd(project_mapper);
            return config_cmd.execute(subcommand_args);
        } else if (command == "completion") {
            return handle_completion(project_mapper, subcommand_args);
        } else if (command == "version") {
            show_version();
            return 0;
        } else {
            std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
            std::cerr << "Run '" << PROGRAM_NAME << " --help' for usage information." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Error: An unexpected error occurred" << std::endl;
        return 1;
    }

    return 0;
}
