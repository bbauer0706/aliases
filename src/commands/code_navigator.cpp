#include "aliases/commands/code_navigator.h"
#include "aliases/process_utils.h"
#include "aliases/file_utils.h"
#include <iostream>
#include <algorithm>

namespace aliases::commands {

CodeNavigator::CodeNavigator(std::shared_ptr<ProjectMapper> mapper)
    : project_mapper_(std::move(mapper)) {}

int CodeNavigator::execute(const StringVector& args) {
    // Handle help
    if (!args.empty() && (args[0] == "-h" || args[0] == "--help")) {
        show_help();
        return 0;
    }
    
    // No parameters, open home directory
    if (args.empty()) {
        open_home_directory();
        return 0;
    }
    
    // Multiple parameters, open each project
    if (args.size() > 1) {
        open_multiple_projects(args);
        return 0;
    }
    
    // Single parameter
    open_project(args[0]);
    return 0;
}

void CodeNavigator::show_help() const {
    std::cout << "VS Code project navigation:" << std::endl;
    std::cout << "  c                - Open home directory" << std::endl;
    std::cout << "  c <project>      - Open project" << std::endl;
    std::cout << "  c <project>s     - Open server component of project" << std::endl;
    std::cout << "  c <project>w     - Open web component of project" << std::endl;
    std::cout << "  c <project>[sw]  - Open both server and web components" << std::endl;
    std::cout << "  c <project>ws    - Open web and server components (shorthand)" << std::endl;
    std::cout << "  c <project>sw    - Open server and web components (shorthand)" << std::endl;
    std::cout << "  c <proj1> <proj2> ... - Open multiple projects" << std::endl;
}

void CodeNavigator::open_home_directory() const {
    launch_vscode(get_home_directory());
}

void CodeNavigator::open_project(const std::string& project_spec) const {
    // Check for bracket notation first
    if (project_spec.find('[') != std::string::npos && project_spec.find(']') != std::string::npos) {
        open_bracket_notation(project_spec);
        return;
    }
    
    auto [base, suffix] = parse_project_spec(project_spec);
    
    // Check if the exact input matches a project first
    if (project_mapper_->get_project_path(project_spec)) {
        // Full input is a valid project
        open_main_project(project_spec);
        return;
    }
    
    // Check if base is valid project when we have a suffix
    auto project_path = project_mapper_->get_project_path(base);
    if (!project_path) {
        // Try to parse as composite project names (e.g., "dipws" -> "dipw" + "dips")
        auto composite_projects = parse_composite_projects(project_spec);
        if (!composite_projects.empty()) {
            open_multiple_projects(composite_projects);
            return;
        }
        
        std::cout << "Unknown project: " << base << std::endl;
        show_available_projects();
        return;
    }
    
    // Handle different cases
    if (suffix.empty()) {
        open_main_project(base);
    } else if (suffix == "s") {
        open_server_component(base);
    } else if (suffix == "w") {
        open_web_component(base);
    }
}

void CodeNavigator::open_multiple_projects(const StringVector& project_specs) const {
    for (const auto& spec : project_specs) {
        open_project(spec);
    }
}

void CodeNavigator::open_main_project(const std::string& project_name) const {
    auto project_path = project_mapper_->get_project_path(project_name);
    if (project_path) {
        std::cout << "Opening project: " << project_name << " (" << *project_path << ")" << std::endl;
        launch_vscode(*project_path);
    }
}

void CodeNavigator::open_server_component(const std::string& project_name) const {
    auto server_path = project_mapper_->get_component_path(project_name, ComponentType::SERVER);
    auto project_path = project_mapper_->get_project_path(project_name);
    
    if (server_path && project_path) {
        auto full_path = FileUtils::join_path(*project_path, *server_path);
        std::cout << "Opening server component: " << project_name 
                  << " (" << Colors::SERVER << full_path << Colors::RESET << ")" << std::endl;
        launch_vscode(full_path);
    } else {
        std::cout << "No server component found for project " << project_name << std::endl;
    }
}

void CodeNavigator::open_web_component(const std::string& project_name) const {
    auto web_path = project_mapper_->get_component_path(project_name, ComponentType::WEB);
    auto project_path = project_mapper_->get_project_path(project_name);
    
    if (web_path && project_path) {
        auto full_path = FileUtils::join_path(*project_path, *web_path);
        std::cout << "Opening web component: " << project_name 
                  << " (" << Colors::WEB << full_path << Colors::RESET << ")" << std::endl;
        launch_vscode(full_path);
    } else {
        std::cout << "No web component found for project " << project_name << std::endl;
    }
}

void CodeNavigator::open_bracket_notation(const std::string& project_spec) const {
    // Parse project[sw] notation
    auto bracket_start = project_spec.find('[');
    auto bracket_end = project_spec.find(']');
    
    if (bracket_start == std::string::npos || bracket_end == std::string::npos) {
        return;
    }
    
    auto project_name = project_spec.substr(0, bracket_start);
    auto variants = project_spec.substr(bracket_start + 1, bracket_end - bracket_start - 1);
    
    // Open each variant
    for (char variant : variants) {
        if (variant == 's') {
            open_server_component(project_name);
        } else if (variant == 'w') {
            open_web_component(project_name);
        }
    }
}

void CodeNavigator::show_available_projects() const {
    std::cout << "Available projects:" << std::endl;
    
    auto projects = project_mapper_->get_all_projects();
    
    // Sort projects by display name
    std::sort(projects.begin(), projects.end(), [](const auto& a, const auto& b) {
        return a.display_name < b.display_name;
    });
    
    for (const auto& project : projects) {
        std::cout << "  ";
        
        // Show display name and full name if different
        if (project.display_name != project.full_name) {
            std::cout << project.display_name << " (" << project.full_name << ") ";
        } else {
            std::cout << project.display_name << " ";
        }
        
        // Show available components
        if (project.has_server_component) {
            std::cout << "| " << Colors::SERVER << project.display_name << "s" << Colors::RESET << " ";
        }
        
        if (project.has_web_component) {
            std::cout << "| " << Colors::WEB << project.display_name << "w" << Colors::RESET << " ";
        }
        
        std::cout << std::endl;
    }
}

std::pair<std::string, std::string> CodeNavigator::parse_project_spec(const std::string& spec) const {
    if (spec.empty()) return {"", ""};
    
    char last_char = spec.back();
    if (last_char == 's' || last_char == 'w') {
        auto base = spec.substr(0, spec.length() - 1);
        // Only treat as suffix if base is a valid project
        if (project_mapper_->get_project_path(base)) {
            return {base, std::string(1, last_char)};
        }
    }
    
    return {spec, ""};
}

StringVector CodeNavigator::parse_composite_projects(const std::string& composite_spec) const {
    auto all_projects = project_mapper_->get_all_projects();
    
    // Strategy: Try to parse as <project><suffixes> where suffixes can be combinations of 'w' and 's'
    // For example: "dipws" -> "dip" + "ws" -> ["dipw", "dips"]
    //              "dipsw" -> "dip" + "sw" -> ["dips", "dipw"]
    
    // Try each project as a potential base
    for (const auto& project : all_projects) {
        StringVector base_names = {project.display_name};
        if (project.display_name != project.full_name) {
            base_names.push_back(project.full_name);
        }
        
        for (const auto& base_name : base_names) {
            if (composite_spec.length() > base_name.length() && 
                composite_spec.substr(0, base_name.length()) == base_name) {
                
                std::string suffixes = composite_spec.substr(base_name.length());
                
                // Check if suffixes are valid combinations of 'w' and 's'
                bool valid_suffixes = true;
                StringVector components;
                
                for (char suffix : suffixes) {
                    if (suffix == 'w') {
                        if (project.has_web_component) {
                            components.push_back(base_name + "w");
                        } else {
                            valid_suffixes = false;
                            break;
                        }
                    } else if (suffix == 's') {
                        if (project.has_server_component) {
                            components.push_back(base_name + "s");
                        } else {
                            valid_suffixes = false;
                            break;
                        }
                    } else {
                        valid_suffixes = false;
                        break;
                    }
                }
                
                if (valid_suffixes && components.size() >= 2) {
                    return components;
                }
            }
        }
    }
    
    return {};
}

void CodeNavigator::launch_vscode(const std::string& path) const {
    ProcessUtils::execute("code " + ProcessUtils::escape_shell_argument(path));
}

} // namespace aliases::commands
