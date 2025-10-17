#include "aliases/project_mapper.h"
#include "aliases/common.h"
#include "aliases/config.h"
#include "aliases/file_utils.h"

namespace aliases {

class ProjectMapper::Impl {
public:
    StringMap full_paths_;
    StringMap full_to_short_;
    StringMap server_paths_;
    StringMap web_paths_;
    StringVector default_server_paths_;
    StringVector default_web_paths_;

    bool initialize() {
        // Get config instance
        auto& config = Config::instance();

        // Get ignore patterns from config
        auto ignore_patterns = config.get_workspace_ignore();

        // Discover all projects from all workspace directories
        auto workspace_dirs = config.get_workspace_directories();

        full_paths_.clear();
        for (const auto& workspace_dir : workspace_dirs) {
            // Expand ~ to home directory
            std::string expanded_dir = workspace_dir;
            if (!expanded_dir.empty() && expanded_dir[0] == '~') {
                expanded_dir = get_home_directory() + expanded_dir.substr(1);
            }

            auto project_dirs = FileUtils::discover_workspace_projects(expanded_dir, ignore_patterns);

            for (const auto& dir : project_dirs) {
                auto full_name = FileUtils::get_basename(dir);
                full_paths_[full_name] = dir;
            }
        }

        // Load project mappings from config
        config.get_project_shortcuts(full_to_short_);
        config.get_server_paths(server_paths_);
        config.get_web_paths(web_paths_);

        // Load default paths
        default_server_paths_ = config.get_default_server_paths();
        default_web_paths_ = config.get_default_web_paths();

        return true;
    }

    std::optional<std::string> get_full_project_name(const std::string& name_or_short) const {
        // Check if it's already a full name
        if (full_paths_.find(name_or_short) != full_paths_.end()) {
            return name_or_short;
        }

        // Check if it's a short name
        for (const auto& [full_name, short_name] : full_to_short_) {
            if (short_name == name_or_short) {
                return full_name;
            }
        }

        return std::nullopt;
    }

    std::string get_display_name(const std::string& full_name) const {
        auto it = full_to_short_.find(full_name);
        return (it != full_to_short_.end()) ? it->second : full_name;
    }

    std::optional<std::string> get_component_path(const std::string& project_name, ComponentType type) const {
        auto full_name = get_full_project_name(project_name);
        if (!full_name)
            return std::nullopt;

        auto project_path_it = full_paths_.find(*full_name);
        if (project_path_it == full_paths_.end())
            return std::nullopt;

        const std::string& project_path = project_path_it->second;

        // Check custom paths first
        if (type == ComponentType::SERVER) {
            auto it = server_paths_.find(*full_name);
            if (it != server_paths_.end()) {
                return it->second;
            }
            // Try default server paths
            return FileUtils::find_component_directory(project_path, default_server_paths_);
        } else if (type == ComponentType::WEB) {
            auto it = web_paths_.find(*full_name);
            if (it != web_paths_.end()) {
                return it->second;
            }
            // Try default web paths
            return FileUtils::find_component_directory(project_path, default_web_paths_);
        }

        return std::nullopt;
    }
};

ProjectMapper::ProjectMapper() : pimpl_(std::make_unique<Impl>()) {}

ProjectMapper::~ProjectMapper() = default;

bool ProjectMapper::initialize() { return pimpl_->initialize(); }

std::vector<ProjectInfo> ProjectMapper::get_all_projects() const {
    std::vector<ProjectInfo> projects;

    for (const auto& [full_name, path] : pimpl_->full_paths_) {
        ProjectInfo info;
        info.full_name = full_name;
        info.display_name = get_display_name(full_name);
        info.path = path;
        info.server_path = get_component_path(full_name, ComponentType::SERVER);
        info.web_path = get_component_path(full_name, ComponentType::WEB);
        info.has_server_component = info.server_path.has_value();
        info.has_web_component = info.web_path.has_value();

        projects.push_back(std::move(info));
    }

    return projects;
}

std::optional<ProjectInfo> ProjectMapper::get_project_info(const std::string& name_or_short) const {
    auto full_name = pimpl_->get_full_project_name(name_or_short);
    if (!full_name)
        return std::nullopt;

    auto path_it = pimpl_->full_paths_.find(*full_name);
    if (path_it == pimpl_->full_paths_.end())
        return std::nullopt;

    ProjectInfo info;
    info.full_name = *full_name;
    info.display_name = get_display_name(*full_name);
    info.path = path_it->second;
    info.server_path = get_component_path(*full_name, ComponentType::SERVER);
    info.web_path = get_component_path(*full_name, ComponentType::WEB);
    info.has_server_component = info.server_path.has_value();
    info.has_web_component = info.web_path.has_value();

    return info;
}

std::optional<std::string> ProjectMapper::get_project_path(const std::string& name_or_short) const {
    auto full_name = pimpl_->get_full_project_name(name_or_short);
    if (!full_name)
        return std::nullopt;

    auto it = pimpl_->full_paths_.find(*full_name);
    return (it != pimpl_->full_paths_.end()) ? std::optional<std::string>(it->second) : std::nullopt;
}

std::optional<std::string> ProjectMapper::get_full_project_name(const std::string& name_or_short) const {
    return pimpl_->get_full_project_name(name_or_short);
}

std::string ProjectMapper::get_display_name(const std::string& full_name) const {
    return pimpl_->get_display_name(full_name);
}

std::optional<std::string> ProjectMapper::get_component_path(const std::string& project_name,
                                                             ComponentType type) const {
    return pimpl_->get_component_path(project_name, type);
}

bool ProjectMapper::has_component(const std::string& project_name, ComponentType type) const {
    return get_component_path(project_name, type).has_value();
}

bool ProjectMapper::reload() { return pimpl_->initialize(); }

} // namespace aliases
