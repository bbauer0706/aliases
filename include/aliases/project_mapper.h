#pragma once

#include "common.h"
#include <memory>

namespace aliases {

class ProjectMapper {
public:
    ProjectMapper();
    ~ProjectMapper();

    // Initialize and load all mappings
    bool initialize();
    
    // Project discovery and mapping
    std::vector<ProjectInfo> get_all_projects() const;
    std::optional<ProjectInfo> get_project_info(const std::string& name_or_short) const;
    std::optional<std::string> get_project_path(const std::string& name_or_short) const;
    
    // Name resolution
    std::optional<std::string> get_full_project_name(const std::string& name_or_short) const;
    std::string get_display_name(const std::string& full_name) const;
    
    // Component path resolution
    std::optional<std::string> get_component_path(const std::string& project_name, ComponentType type) const;
    bool has_component(const std::string& project_name, ComponentType type) const;
    
    // Reload mappings (useful for config changes)
    bool reload();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace aliases
