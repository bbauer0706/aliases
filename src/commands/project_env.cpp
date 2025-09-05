#include "aliases/commands/project_env.h"
#include "aliases/process_utils.h"
#include "aliases/file_utils.h"
#include "aliases/common.h"
#include <iostream>

namespace aliases::commands {

ProjectEnv::ProjectEnv(std::shared_ptr<ProjectMapper> mapper)
    : project_mapper_(std::move(mapper)) {}

int ProjectEnv::execute(const StringVector& args) {
    if (!args.empty() && (args[0] == "-h" || args[0] == "--help")) {
        show_help();
        return 0;
    }
    
    if (!args.empty() && args[0] == "--show") {
        show_environment_variables();
        return 0;
    }
    
    auto config = parse_arguments(args);
    auto env = setup_project_environment(config);
    export_environment_variables(env);
    print_success_message(env);
    
    return 0;
}

EnvironmentConfig ProjectEnv::parse_arguments(const StringVector& args) {
    EnvironmentConfig config;
    
    for (size_t i = 0; i < args.size(); ++i) {
        if ((args[i] == "-e") && i + 1 < args.size()) {
            config.profile = args[++i];
        } else if ((args[i] == "-s") && i + 1 < args.size()) {
            config.use_https = (args[++i] == "true");
        } else if ((args[i] == "-p") && i + 1 < args.size()) {
            config.starting_port = std::stoi(args[++i]);
        } else if ((args[i] == "-i") && i + 1 < args.size()) {
            config.introspection = (args[++i] == "true");
        } else if ((args[i] == "-t") && i + 1 < args.size()) {
            config.transfer_mode = args[++i];
        } else if (args[i] == "-n") {
            config.no_port_offset = true;
        }
    }
    
    return config;
}

void ProjectEnv::show_help() const {
    std::cout << "Usage: project_env [OPTIONS]" << std::endl;
    std::cout << "Sets up environment variables for project development." << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -e ENV      Set environment profile (dev, prod, etc). Default: dev" << std::endl;
    std::cout << "  -s FLAG     Enable/disable HTTPS (true/false). Default: false" << std::endl;
    std::cout << "  -p PORT     Starting port number to check availability. Default: 3000" << std::endl;
    std::cout << "  -i FLAG     Enable/disable GraphQL introspection (true/false). Default: true" << std::endl;
    std::cout << "  -t MODE     Set transfer mode (plain, compressed, etc). Default: plain" << std::endl;
    std::cout << "  -n          No port offset - use same port for WEB and GQL services" << std::endl;
    std::cout << "  --show      Display current environment variables and exit" << std::endl;
    std::cout << "  -h, --help  Display this help message and exit" << std::endl;
}

void ProjectEnv::show_environment_variables() const {
    std::cout << "Current Project Environment Variables:" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    
    const char* env_vars[] = {
        "PROJECT_NAME", "PROFILE", "GQLHOST", "WEBPORT", "GQLPORT", 
        "SBPORT", "NDEBUGPORT", "GQLNUMBEROFMAXRETRIES", "GQLSERVERPATH",
        "GQLHTTPS", "GQLINTROSPECTION", "GQLTRANSFERMODE"
    };
    
    for (const char* var : env_vars) {
        const char* value = std::getenv(var);
        std::cout << var << ": " << (value ? value : "Not set") << std::endl;
    }
    
    std::cout << "------------------------------------" << std::endl;
}

ProjectEnvironment ProjectEnv::setup_project_environment(const EnvironmentConfig& config) {
    ProjectEnvironment env;
    
    // Get project name from current directory
    env.project_name = get_project_name_from_directory();
    env.profile = config.profile;
    env.gql_https = config.use_https;
    env.gql_introspection = config.introspection;
    env.gql_transfer_mode = config.transfer_mode;
    
    // Get hostname
    env.gql_host = get_current_hostname();
    
    // Calculate ports
    int project_offset = get_project_port_offset(env.project_name);
    int base_port = config.starting_port + project_offset;
    
    bool is_server_dir = is_server_directory();
    
    if (config.no_port_offset) {
        env.web_port = find_available_port(base_port, false);
        env.gql_port = env.web_port;
    } else {
        env.web_port = find_available_port(base_port, is_server_dir);
        env.gql_port = env.web_port + 1;
    }
    
    env.sb_port = env.web_port + 2;
    env.ndebug_port = env.web_port + 3;
    
    return env;
}

void ProjectEnv::export_environment_variables(const ProjectEnvironment& env) {
    // Output shell commands that can be eval'ed by the calling shell
    std::cout << "export PROJECT_NAME='" << env.project_name << "';" << std::endl;
    std::cout << "export PROFILE='" << env.profile << "';" << std::endl;
    std::cout << "export GQLHOST='" << env.gql_host << "';" << std::endl;
    std::cout << "export WEBPORT=" << env.web_port << ";" << std::endl;
    std::cout << "export GQLPORT=" << env.gql_port << ";" << std::endl;
    std::cout << "export SBPORT=" << env.sb_port << ";" << std::endl;
    std::cout << "export NDEBUGPORT=" << env.ndebug_port << ";" << std::endl;
    std::cout << "export GQLNUMBEROFMAXRETRIES=" << env.gql_max_retries << ";" << std::endl;
    std::cout << "export GQLSERVERPATH='" << env.gql_server_path << "';" << std::endl;
    std::cout << "export GQLHTTPS=" << (env.gql_https ? "true" : "false") << ";" << std::endl;
    std::cout << "export GQLINTROSPECTION=" << (env.gql_introspection ? "true" : "false") << ";" << std::endl;
    std::cout << "export GQLTRANSFERMODE='" << env.gql_transfer_mode << "';" << std::endl;
}

std::string ProjectEnv::get_project_name_from_directory() const {
    auto current_dir = get_current_directory();
    auto workspace_dir = get_workspace_directory();
    
    if (starts_with(current_dir, workspace_dir + "/")) {
        auto relative_path = current_dir.substr(workspace_dir.length() + 1);
        auto first_slash = relative_path.find('/');
        return (first_slash != std::string::npos) ? relative_path.substr(0, first_slash) : relative_path;
    }
    
    return FileUtils::get_basename(current_dir);
}

bool ProjectEnv::is_server_directory() const {
    if (!project_mapper_) return false;
    
    auto project_name = get_project_name_from_directory();
    auto project_info = project_mapper_->get_project_info(project_name);
    
    if (!project_info || !project_info->has_server_component) {
        return false;
    }
    
    // Check if current directory is within the server component path
    auto current_dir = get_current_directory();
    auto server_path = project_info->server_path;
    
    return server_path && starts_with(current_dir, *server_path);
}

int ProjectEnv::get_project_port_offset(const std::string& project_name) const {
    // Simple hash-based offset calculation
    std::hash<std::string> hasher;
    auto hash_val = hasher(project_name);
    return 100 + (hash_val % 90) * 10;
}

int ProjectEnv::find_available_port(int starting_port, bool is_server_dir) const {
    int port_to_check = starting_port;
    
    while (!is_port_available(port_to_check)) {
        // For server directories, if starting port is occupied but next port is available,
        // still return the starting port (server needs specific ports to match frontend)
        if (is_server_dir && port_to_check == starting_port && is_port_available(port_to_check + 1)) {
            return starting_port;
        }
        port_to_check++;
    }
    
    return port_to_check;
}

bool ProjectEnv::is_port_available(int port) const {
    return ProcessUtils::is_port_available(port);
}

std::string ProjectEnv::get_current_hostname() const {
    auto result = ProcessUtils::execute("hostname");
    return result.success() ? trim(result.stdout_output) : "localhost";
}

void ProjectEnv::print_success_message(const ProjectEnvironment& env) const {
    std::cerr << Colors::SUCCESS << "[SUCCESS]" << Colors::RESET 
              << " Project environment loaded for: " << env.project_name 
              << ", PORT: " << env.web_port 
              << ", MODE: " << env.gql_transfer_mode << std::endl;
}

} // namespace aliases::commands
