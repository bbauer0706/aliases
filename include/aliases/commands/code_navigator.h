#pragma once

#include "../common.h"
#include "../project_mapper.h"

namespace aliases::commands {

class CodeNavigator {
public:
    explicit CodeNavigator(std::shared_ptr<ProjectMapper> mapper);
    ~CodeNavigator() = default;

    // Main command entry point
    int execute(const StringVector& args);

private:
    std::shared_ptr<ProjectMapper> project_mapper_;
    
    // Command implementations
    void show_help() const;
    void open_home_directory() const;
    void open_project(const std::string& project_spec) const;
    void open_multiple_projects(const StringVector& project_specs) const;
    
    // Project opening logic
    void open_main_project(const std::string& project_name) const;
    void open_server_component(const std::string& project_name) const;
    void open_web_component(const std::string& project_name) const;
    void open_bracket_notation(const std::string& project_spec) const;
    
    // Utility functions
    void show_available_projects() const;
    bool is_valid_project_spec(const std::string& spec) const;
    std::pair<std::string, std::string> parse_project_spec(const std::string& spec) const;
    StringVector parse_composite_projects(const std::string& composite_spec) const;
    
    // VS Code integration
    void launch_vscode(const std::string& path) const;
};

} // namespace aliases::commands
