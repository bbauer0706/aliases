#pragma once

#include "../common.h"
#include "../project_mapper.h"

namespace aliases::commands {

struct EnvironmentConfig {
    std::string profile = "dev";
    bool use_https = false;
    int starting_port = 3000;
    bool introspection = true;
    std::string transfer_mode = "plain";
    bool no_port_offset = false;
};

struct ProjectEnvironment {
    std::string project_name;
    std::string profile;
    std::string gql_host;
    int web_port = 0;
    int gql_port = 0;
    int sb_port = 0;
    int ndebug_port = 0;
    int gql_max_retries = 3;
    std::string gql_server_path = "/graphql";
    bool gql_https = false;
    bool gql_introspection = true;
    std::string gql_transfer_mode = "plain";
};

class ProjectEnv {
public:
    explicit ProjectEnv(std::shared_ptr<ProjectMapper> mapper);
    ~ProjectEnv() = default;

    // Main command entry point
    int execute(const StringVector& args);

private:
    std::shared_ptr<ProjectMapper> project_mapper_;
    
    // Command parsing and help
    EnvironmentConfig parse_arguments(const StringVector& args);
    void show_help() const;
    void show_environment_variables() const;
    
    // Environment setup
    ProjectEnvironment setup_project_environment(const EnvironmentConfig& config);
    void export_environment_variables(const ProjectEnvironment& env);
    
    // Project detection
    std::string get_project_name_from_directory() const;
    bool is_server_directory() const;
    int get_project_port_offset(const std::string& project_name) const;
    
    // Port management
    int find_available_port(int starting_port, bool is_server_dir) const;
    bool is_port_available(int port) const;
    
    // Auto-update functionality
    void auto_update_project_env();
    void setup_new_terminal();
    
    // Utility functions
    std::string get_current_hostname() const;
    void print_success_message(const ProjectEnvironment& env) const;
};

} // namespace aliases::commands
