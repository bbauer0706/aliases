#pragma once

#include "../common.h"
#include "../project_mapper.h"
#include "../git_operations.h"
#include <future>

namespace aliases::commands {

struct UpdateConfig {
    int max_parallel_jobs = 4;
    bool verbose = false;
    StringVector projects_to_update;
};

struct UpdateStats {
    int total_projects = 0;
    int successful_updates = 0;
    int failed_updates = 0;
    int skipped_projects = 0;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
};

class WorkspaceUpdater {
public:
    explicit WorkspaceUpdater(std::shared_ptr<ProjectMapper> mapper);
    ~WorkspaceUpdater() = default;

    // Main command entry point
    int execute(const StringVector& args);

private:
    std::shared_ptr<ProjectMapper> project_mapper_;
    
    // Command parsing and help
    UpdateConfig parse_arguments(const StringVector& args);
    void show_help() const;
    
    // Update orchestration
    UpdateStats update_projects(const UpdateConfig& config);
    void update_project_async(
        const std::string& project_name,
        const std::string& component_suffix,
        std::promise<bool> result_promise
    );
    
    // Individual project update logic
    bool update_project_component(
        const std::string& project_name,
        ComponentType component_type,
        const std::string& component_path = ""
    );
    
    // Package management
    bool update_maven_dependencies(const std::string& directory);
    bool update_npm_packages(const std::string& directory);
    bool detect_and_update_packages(const std::string& directory, ComponentType type);
    
    // Progress and logging
    void log_update_status(const std::string& level, const std::string& project, const std::string& message) const;
    void show_update_summary(const UpdateStats& stats) const;
    
    // Utility functions
    std::pair<std::string, std::string> parse_project_spec(const std::string& spec) const;
    void wait_for_available_slot(std::vector<std::future<bool>>& futures, int max_jobs);
};

} // namespace aliases::commands
