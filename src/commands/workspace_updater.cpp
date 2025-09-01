#include "aliases/commands/workspace_updater.h"
#include "aliases/process_utils.h"
#include "aliases/file_utils.h"
#include "aliases/git_operations.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <filesystem>

namespace aliases::commands {

WorkspaceUpdater::WorkspaceUpdater(std::shared_ptr<ProjectMapper> mapper)
    : project_mapper_(std::move(mapper)) {}

int WorkspaceUpdater::execute(const StringVector& args) {
    if (!args.empty() && (args[0] == "-h" || args[0] == "--help")) {
        show_help();
        return 0;
    }
    
    auto config = parse_arguments(args);
    auto stats = update_projects(config);
    show_update_summary(stats);
    
    return (stats.failed_updates > 0) ? 1 : 0;
}

UpdateConfig WorkspaceUpdater::parse_arguments(const StringVector& args) {
    UpdateConfig config;
    
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-j" || args[i] == "--jobs") {
            if (i + 1 < args.size()) {
                try {
                    config.max_parallel_jobs = std::stoi(args[i + 1]);
                    if (config.max_parallel_jobs <= 0) {
                        config.max_parallel_jobs = 4; // Default fallback
                    }
                } catch (const std::exception&) {
                    std::cerr << "Warning: Invalid job count '" << args[i + 1] << "', using default (4)" << std::endl;
                    config.max_parallel_jobs = 4;
                }
                ++i; // Skip next argument
            }
        } else {
            config.projects_to_update.push_back(args[i]);
        }
    }
    
    // If no projects specified, update all
    if (config.projects_to_update.empty()) {
        auto all_projects = project_mapper_->get_all_projects();
        for (const auto& project : all_projects) {
            config.projects_to_update.push_back(project.full_name);
        }
    }
    
    return config;
}

void WorkspaceUpdater::show_help() const {
    std::cout << "Workspace update utility:" << std::endl;
    std::cout << "  uw                    - Update all projects" << std::endl;
    std::cout << "  uw <project>          - Update specific project" << std::endl;
    std::cout << "  uw <project>s         - Update server component only" << std::endl;
    std::cout << "  uw <project>w         - Update web component only" << std::endl;
    std::cout << "  uw <proj1> <proj2>... - Update multiple specific projects" << std::endl;
    std::cout << "  uw -j <num>           - Set max parallel jobs (default: 4)" << std::endl;
    std::cout << std::endl;
    std::cout << "The script will:" << std::endl;
    std::cout << "  1. Skip projects with uncommitted changes" << std::endl;
    std::cout << "  2. Switch to main branch (if not already on main)" << std::endl;
    std::cout << "  3. Pull latest changes" << std::endl;
    std::cout << "  4. Update packages (Maven for server, npm for web)" << std::endl;
    std::cout << "  5. Switch back to original branch" << std::endl;
}

UpdateStats WorkspaceUpdater::update_projects(const UpdateConfig& config) {
    UpdateStats stats;
    stats.total_projects = config.projects_to_update.size();
    stats.start_time = std::chrono::steady_clock::now();
    
    std::cout << "Starting workspace update with " << config.max_parallel_jobs << " parallel jobs..." << std::endl;
    std::cout << "Projects to process: " << stats.total_projects << std::endl;
    std::cout << "==============================================" << std::endl;
    
    // Parallel processing implementation
    std::vector<std::future<bool>> futures;
    futures.reserve(config.projects_to_update.size() * 3); // Worst case: main + server + web per project
    
    for (const auto& project_spec : config.projects_to_update) {
        auto [base_project, suffix] = parse_project_spec(project_spec);
        
        if (!project_mapper_->get_project_path(base_project)) {
            log_update_status("ERROR", project_spec, "Unknown project");
            stats.failed_updates++;
            continue;
        }
        
        // Wait for available slot before launching new tasks
        wait_for_available_slot(futures, config.max_parallel_jobs);
        
        if (suffix.empty()) {
            // Launch async task for main project and components
            futures.push_back(std::async(std::launch::async, [this, base_project]() {
                bool success = update_project_component(base_project, ComponentType::MAIN);
                
                // Also update components if they exist
                if (project_mapper_->has_component(base_project, ComponentType::SERVER)) {
                    auto server_path = project_mapper_->get_component_path(base_project, ComponentType::SERVER);
                    if (server_path) {
                        success &= update_project_component(base_project, ComponentType::SERVER, *server_path);
                    }
                }
                
                if (project_mapper_->has_component(base_project, ComponentType::WEB)) {
                    auto web_path = project_mapper_->get_component_path(base_project, ComponentType::WEB);
                    if (web_path) {
                        success &= update_project_component(base_project, ComponentType::WEB, *web_path);
                    }
                }
                
                return success;
            }));
        } else if (suffix == "s") {
            auto server_path = project_mapper_->get_component_path(base_project, ComponentType::SERVER);
            if (server_path) {
                futures.push_back(std::async(std::launch::async, [this, base_project, server_path]() {
                    return update_project_component(base_project, ComponentType::SERVER, *server_path);
                }));
            } else {
                log_update_status("ERROR", project_spec, "No server component found");
                stats.failed_updates++;
                continue;
            }
        } else if (suffix == "w") {
            auto web_path = project_mapper_->get_component_path(base_project, ComponentType::WEB);
            if (web_path) {
                futures.push_back(std::async(std::launch::async, [this, base_project, web_path]() {
                    return update_project_component(base_project, ComponentType::WEB, *web_path);
                }));
            } else {
                log_update_status("ERROR", project_spec, "No web component found");
                stats.failed_updates++;
                continue;
            }
        }
    }
    
    // Wait for all tasks to complete and collect results
    for (auto& future : futures) {
        if (future.get()) {
            stats.successful_updates++;
        } else {
            stats.failed_updates++;
        }
    }
    
    stats.end_time = std::chrono::steady_clock::now();
    return stats;
}

bool WorkspaceUpdater::update_project_component(
    const std::string& project_name,
    ComponentType component_type,
    const std::string& component_path
) {
    auto project_path = project_mapper_->get_project_path(project_name);
    if (!project_path) {
        log_update_status("ERROR", project_name, "Project path not found");
        return false;
    }
    
    std::string full_path = *project_path;
    std::string component_name = project_name;
    
    if (component_type != ComponentType::MAIN) {
        full_path = *project_path + "/" + component_path;
        component_name += (component_type == ComponentType::SERVER) ? "-server" : "-web";
    }
    
    // Check if directory exists
    if (!std::filesystem::exists(full_path)) {
        log_update_status("ERROR", component_name, "Directory does not exist: " + full_path);
        return false;
    }
    
    // Get git status
    auto git_status = GitOperations::get_git_status(full_path);
    if (!git_status.is_git_repo) {
        log_update_status("SKIPPED", component_name, "Not a git repository");
        return true; // Not an error, just skip
    }
    
    // Check for uncommitted changes
    if (git_status.has_uncommitted_changes) {
        log_update_status("SKIPPED", component_name, "Has uncommitted changes");
        return true; // Not an error, just skip
    }
    
    log_update_status("INFO", component_name, "Starting update (current branch: " + git_status.current_branch + ")");
    
    // Switch to main branch if not already on main
    bool switched_branch = false;
    std::string original_branch = git_status.current_branch;
    if (!git_status.is_main_branch) {
        log_update_status("INFO", component_name, "Switching to main branch");
        auto main_branch = GitOperations::get_main_branch_name(full_path);
        auto checkout_result = GitOperations::checkout_branch(full_path, main_branch);
        if (!checkout_result) {
            log_update_status("ERROR", component_name, "Failed to switch to main branch: " + checkout_result.error_message);
            return false;
        }
        switched_branch = true;
    }
    
    // Pull latest changes
    log_update_status("INFO", component_name, "Pulling latest changes");
    auto pull_result = GitOperations::pull_changes(full_path);
    if (!pull_result) {
        log_update_status("ERROR", component_name, "Failed to pull changes: " + pull_result.error_message);
        // Try to switch back to original branch if we switched
        if (switched_branch) {
            GitOperations::checkout_branch(full_path, original_branch);
        }
        return false;
    }
    
    // Update packages based on component type
    bool update_success = detect_and_update_packages(full_path, component_type);
    
    // Switch back to original branch if we switched
    if (switched_branch) {
        log_update_status("INFO", component_name, "Switching back to " + original_branch);
        auto checkout_result = GitOperations::checkout_branch(full_path, original_branch);
        if (!checkout_result) {
            log_update_status("ERROR", component_name, "Failed to switch back to " + original_branch);
            return false;
        }
    }
    
    if (update_success) {
        log_update_status("SUCCESS", component_name, "Update completed successfully");
    } else {
        log_update_status("WARNING", component_name, "Update completed with warnings");
    }
    
    return update_success;
}

void WorkspaceUpdater::log_update_status(
    const std::string& level, 
    const std::string& project, 
    const std::string& message
) const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    printf("[%02d:%02d:%02d] ", tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    if (level == "SUCCESS") {
        std::cout << Colors::SUCCESS << "✓" << Colors::RESET;
    } else if (level == "ERROR") {
        std::cout << Colors::ERROR << "✗" << Colors::RESET;
    } else if (level == "WARNING") {
        std::cout << Colors::WARNING << "⚠" << Colors::RESET;
    } else if (level == "INFO") {
        std::cout << Colors::INFO << "ℹ" << Colors::RESET;
    } else if (level == "SKIPPED") {
        std::cout << Colors::SKIPPED << "⊘" << Colors::RESET;
    }
    
    std::cout << " " << project << ": " << message << std::endl;
}

void WorkspaceUpdater::show_update_summary(const UpdateStats& stats) const {
    std::cout << "==============================================" << std::endl;
    std::cout << "Workspace update completed!" << std::endl;
    std::cout << "Total projects: " << stats.total_projects << std::endl;
    std::cout << "Successful: " << stats.successful_updates << std::endl;
    std::cout << "Failed: " << stats.failed_updates << std::endl;
    std::cout << "Skipped: " << stats.skipped_projects << std::endl;
    
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stats.end_time - stats.start_time);
    std::cout << "Duration: " << duration.count() << " seconds" << std::endl;
}

std::pair<std::string, std::string> WorkspaceUpdater::parse_project_spec(const std::string& spec) const {
    if (spec.empty()) return {"", ""};
    
    char last_char = spec.back();
    if (last_char == 's' || last_char == 'w') {
        auto base = spec.substr(0, spec.length() - 1);
        if (project_mapper_->get_project_path(base)) {
            return {base, std::string(1, last_char)};
        }
    }
    
    return {spec, ""};
}

void WorkspaceUpdater::wait_for_available_slot(std::vector<std::future<bool>>& futures, int max_jobs) {
    // Remove completed futures
    futures.erase(
        std::remove_if(futures.begin(), futures.end(), [](auto& future) {
            return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }),
        futures.end()
    );
    
    // Wait if we have too many running
    while (static_cast<int>(futures.size()) >= max_jobs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        futures.erase(
            std::remove_if(futures.begin(), futures.end(), [](auto& future) {
                return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
            }),
            futures.end()
        );
    }
}

bool WorkspaceUpdater::update_maven_dependencies(const std::string& directory) {
    log_update_status("INFO", FileUtils::get_basename(directory), "Updating Maven dependencies");
    auto result = ProcessUtils::execute("mvn dependency:resolve dependency:resolve-sources -q", directory);
    if (!result.success()) {
        log_update_status("WARNING", FileUtils::get_basename(directory), "Maven dependency update failed");
        return false;
    }
    return true;
}

bool WorkspaceUpdater::update_npm_packages(const std::string& directory) {
    log_update_status("INFO", FileUtils::get_basename(directory), "Running npm install");
    auto result = ProcessUtils::execute("npm install --silent", directory);
    if (!result.success()) {
        log_update_status("WARNING", FileUtils::get_basename(directory), "npm install failed");
        return false;
    }
    return true;
}

bool WorkspaceUpdater::detect_and_update_packages(const std::string& directory, ComponentType type) {
    bool overall_success = true;
    
    if (type == ComponentType::SERVER || (type == ComponentType::MAIN && std::filesystem::exists(directory + "/pom.xml"))) {
        if (!update_maven_dependencies(directory)) {
            overall_success = false;
        }
    }
    
    if (type == ComponentType::WEB || (type == ComponentType::MAIN && std::filesystem::exists(directory + "/package.json"))) {
        if (!update_npm_packages(directory)) {
            overall_success = false;
        }
    }
    
    return overall_success;
}

} // namespace aliases::commands
