#include "aliases/commands/workspace_updater.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

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
                config.max_parallel_jobs = std::stoi(args[i + 1]);
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
    
    // For now, implement sequential processing
    // TODO: Implement parallel processing with proper job management
    for (const auto& project_spec : config.projects_to_update) {
        auto [base_project, suffix] = parse_project_spec(project_spec);
        
        if (!project_mapper_->get_project_path(base_project)) {
            log_update_status("ERROR", project_spec, "Unknown project");
            stats.failed_updates++;
            continue;
        }
        
        bool success = false;
        if (suffix.empty()) {
            // Update main project and components
            success = update_project_component(base_project, ComponentType::MAIN);
            
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
        } else if (suffix == "s") {
            auto server_path = project_mapper_->get_component_path(base_project, ComponentType::SERVER);
            if (server_path) {
                success = update_project_component(base_project, ComponentType::SERVER, *server_path);
            } else {
                log_update_status("ERROR", project_spec, "No server component found");
                stats.failed_updates++;
                continue;
            }
        } else if (suffix == "w") {
            auto web_path = project_mapper_->get_component_path(base_project, ComponentType::WEB);
            if (web_path) {
                success = update_project_component(base_project, ComponentType::WEB, *web_path);
            } else {
                log_update_status("ERROR", project_spec, "No web component found");
                stats.failed_updates++;
                continue;
            }
        }
        
        if (success) {
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
    // This is a stub implementation
    // TODO: Implement the actual update logic similar to the bash version
    
    std::string component_name = project_name;
    if (component_type != ComponentType::MAIN) {
        component_name += (component_type == ComponentType::SERVER) ? "-server" : "-web";
    }
    
    log_update_status("INFO", component_name, "Update completed (stub implementation)");
    return true;
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

} // namespace aliases::commands
