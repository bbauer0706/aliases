#pragma once

#include "common.h"

namespace aliases {

struct GitStatus {
    bool is_git_repo = false;
    bool has_uncommitted_changes = false;
    std::string current_branch;
    bool is_main_branch = false;
};

class GitOperations {
public:
    // Git status and branch information
    static GitStatus get_git_status(const std::string& directory);
    static std::string get_current_branch(const std::string& directory);
    static bool has_uncommitted_changes(const std::string& directory);
    static bool is_git_repository(const std::string& directory);
    
    // Git operations
    static Result<std::string> checkout_branch(const std::string& directory, const std::string& branch);
    static Result<std::string> pull_changes(const std::string& directory);
    static bool is_main_branch(const std::string& branch_name);
    
    // Utility functions
    static std::string get_main_branch_name(const std::string& directory);
};

} // namespace aliases
