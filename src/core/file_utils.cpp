#include "aliases/file_utils.h"
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

namespace aliases {

bool FileUtils::directory_exists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}

std::vector<std::string> FileUtils::list_directories(const std::string& path) {
    std::vector<std::string> directories;

    DIR* dir = opendir(path.c_str());
    if (!dir)
        return directories;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR && std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
            directories.push_back(entry->d_name);
        }
    }

    closedir(dir);
    return directories;
}

std::string FileUtils::get_basename(const std::string& path) {
    auto pos = path.find_last_of('/');
    return (pos != std::string::npos) ? path.substr(pos + 1) : path;
}

std::string FileUtils::join_path(const std::string& base, const std::string& relative) {
    if (base.empty())
        return relative;
    if (relative.empty())
        return base;

    bool base_ends_slash = base.back() == '/';
    bool rel_starts_slash = relative.front() == '/';

    if (base_ends_slash && rel_starts_slash) {
        return base + relative.substr(1);
    } else if (!base_ends_slash && !rel_starts_slash) {
        return base + "/" + relative;
    } else {
        return base + relative;
    }
}

bool FileUtils::file_exists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode);
}

std::optional<std::string> FileUtils::read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        return std::nullopt;

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string FileUtils::get_parent_directory(const std::string& path) {
    auto pos = path.find_last_of('/');
    return (pos != std::string::npos) ? path.substr(0, pos) : ".";
}

// Helper function to match glob patterns (* wildcard support)
static bool matches_glob_pattern(const std::string& name, const std::string& pattern) {
    // Simple glob pattern matching supporting * wildcard
    size_t pattern_pos = 0;
    size_t name_pos = 0;
    size_t star_pos = std::string::npos;
    size_t match_pos = 0;

    while (name_pos < name.length()) {
        if (pattern_pos < pattern.length() && pattern[pattern_pos] == '*') {
            // Remember star position
            star_pos = pattern_pos++;
            match_pos = name_pos;
        } else if (pattern_pos < pattern.length() &&
                   (pattern[pattern_pos] == '?' || pattern[pattern_pos] == name[name_pos])) {
            // Match single character or exact match
            pattern_pos++;
            name_pos++;
        } else if (star_pos != std::string::npos) {
            // Backtrack to last star and try matching one more character
            pattern_pos = star_pos + 1;
            name_pos = ++match_pos;
        } else {
            // No match
            return false;
        }
    }

    // Skip remaining stars in pattern
    while (pattern_pos < pattern.length() && pattern[pattern_pos] == '*') {
        pattern_pos++;
    }

    return pattern_pos == pattern.length();
}

std::vector<std::string> FileUtils::discover_workspace_projects(const std::string& workspace_dir) {
    return discover_workspace_projects(workspace_dir, {});
}

std::vector<std::string> FileUtils::discover_workspace_projects(const std::string& workspace_dir,
                                                                const std::vector<std::string>& ignore_patterns) {
    std::vector<std::string> projects;

    if (!directory_exists(workspace_dir)) {
        return projects;
    }

    auto subdirs = list_directories(workspace_dir);
    for (const auto& subdir : subdirs) {
        // Check if directory should be ignored
        bool should_ignore = false;
        for (const auto& pattern : ignore_patterns) {
            // Support both exact match and glob pattern matching
            if (matches_glob_pattern(subdir, pattern)) {
                should_ignore = true;
                break;
            }
        }

        if (should_ignore) {
            continue;
        }

        auto full_path = join_path(workspace_dir, subdir);
        if (directory_exists(full_path)) {
            projects.push_back(full_path);
        }
    }

    return projects;
}

std::optional<std::string> FileUtils::find_component_directory(const std::string& project_path,
                                                               const std::vector<std::string>& candidate_paths) {
    for (const auto& candidate : candidate_paths) {
        auto full_path = join_path(project_path, candidate);
        if (directory_exists(full_path)) {
            return candidate;
        }
    }
    return std::nullopt;
}

} // namespace aliases
