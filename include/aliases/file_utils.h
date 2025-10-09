#pragma once

#include "common.h"

namespace aliases {

class FileUtils {
public:
    // Directory operations
    static bool directory_exists(const std::string& path);
    static std::vector<std::string> list_directories(const std::string& path);
    static std::string get_basename(const std::string& path);
    static std::string join_path(const std::string& base, const std::string& relative);
    
    // File operations
    static bool file_exists(const std::string& path);
    static std::optional<std::string> read_file(const std::string& path);
    
    // Path utilities
    static std::string normalize_path(const std::string& path);
    static std::string get_parent_directory(const std::string& path);
    static std::string resolve_path(const std::string& path);
    
    // Workspace-specific utilities
    static std::vector<std::string> discover_workspace_projects(const std::string& workspace_dir);
    static std::vector<std::string> discover_workspace_projects(
        const std::string& workspace_dir,
        const std::vector<std::string>& ignore_patterns
    );
    static std::optional<std::string> find_component_directory(
        const std::string& project_path,
        const std::vector<std::string>& candidate_paths
    );
};

} // namespace aliases
