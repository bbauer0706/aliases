#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "aliases/project_mapper.h"
#include "aliases/commands/code_navigator.h"
#include "aliases/commands/workspace_updater.h"
#include "aliases/commands/project_env.h"

namespace {
    constexpr const char* VERSION = "1.0.0";
    constexpr const char* PROGRAM_NAME = "aliases-cli";
}

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
    std::cout << "  update, uw       Update workspace projects" << std::endl;
    std::cout << "  env              Setup project environment variables" << std::endl;
    std::cout << "  version          Show version information" << std::endl;
    std::cout << "  help             Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Use '" << PROGRAM_NAME << " <command> --help' for more information on a command." << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << PROGRAM_NAME << " code urm          # Open project 'urm' in VS Code" << std::endl;
    std::cout << "  " << PROGRAM_NAME << " update            # Update all projects" << std::endl;
    std::cout << "  " << PROGRAM_NAME << " env -p 3000       # Setup environment with port 3000" << std::endl;
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
        }
        else if (command == "update" || command == "uw") {
            aliases::commands::WorkspaceUpdater updater(project_mapper);
            return updater.execute(subcommand_args);
        }
        else if (command == "env") {
            aliases::commands::ProjectEnv env_setup(project_mapper);
            return env_setup.execute(subcommand_args);
        }
        else if (command == "version") {
            show_version();
            return 0;
        }
        else {
            std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
            std::cerr << "Run '" << PROGRAM_NAME << " --help' for usage information." << std::endl;
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Error: An unexpected error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
