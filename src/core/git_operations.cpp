#include "aliases/git_operations.h"
#include "aliases/common.h"
#include "aliases/file_utils.h"
#include "aliases/process_utils.h"

namespace aliases {

GitStatus GitOperations::get_git_status(const std::string& directory) {
    GitStatus status;

    // Check if it's a git repository
    auto git_dir = FileUtils::join_path(directory, ".git");
    status.is_git_repo = FileUtils::directory_exists(git_dir);

    if (!status.is_git_repo) {
        return status;
    }

    // Get current branch
    status.current_branch = get_current_branch(directory);
    status.is_main_branch = is_main_branch(status.current_branch);

    // Check for uncommitted changes
    status.has_uncommitted_changes = has_uncommitted_changes(directory);

    return status;
}

std::string GitOperations::get_current_branch(const std::string& directory) {
    auto result = ProcessUtils::execute("git rev-parse --abbrev-ref HEAD", directory);
    if (result.success()) {
        return trim(result.stdout_output);
    }
    return "";
}

bool GitOperations::has_uncommitted_changes(const std::string& directory) {
    auto result = ProcessUtils::execute("git status --porcelain", directory);
    return result.success() && !trim(result.stdout_output).empty();
}

bool GitOperations::is_git_repository(const std::string& directory) {
    auto git_dir = FileUtils::join_path(directory, ".git");
    return FileUtils::directory_exists(git_dir);
}

Result<std::string> GitOperations::checkout_branch(const std::string& directory, const std::string& branch) {
    auto result = ProcessUtils::execute("git checkout " + branch, directory);
    if (result.success()) {
        return Result<std::string>::success_with(std::string("Switched to branch " + branch));
    } else {
        return Result<std::string>::error("Failed to checkout branch: " + result.stderr_output);
    }
}

Result<std::string> GitOperations::pull_changes(const std::string& directory) {
    auto result = ProcessUtils::execute("git pull --ff-only", directory);
    if (result.success()) {
        return Result<std::string>::success_with(std::move(result.stdout_output));
    } else {
        return Result<std::string>::error("Failed to pull changes: " + result.stderr_output);
    }
}

bool GitOperations::is_main_branch(const std::string& branch_name) {
    return branch_name == "main" || branch_name == "master";
}

std::string GitOperations::get_main_branch_name(const std::string& directory) {
    // Try to determine main branch name
    auto result = ProcessUtils::execute("git symbolic-ref refs/remotes/origin/HEAD", directory);
    if (result.success()) {
        auto output = trim(result.stdout_output);
        auto pos = output.find_last_of('/');
        if (pos != std::string::npos) {
            return output.substr(pos + 1);
        }
    }

    // Default fallback
    return "main";
}

} // namespace aliases
