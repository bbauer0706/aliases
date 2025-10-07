#pragma once

#include "common.h"
#include <string>

namespace aliases {

/**
 * Configuration sync manager
 *
 * Handles syncing configuration to/from remote storage
 * Supports multiple sync methods: git, rsync, http, file
 */
class ConfigSync {
public:
    ConfigSync() = default;
    ~ConfigSync() = default;

    // Sync operations
    bool pull();  // Pull config from remote
    bool push();  // Push config to remote
    bool status(); // Show sync status

    // Setup
    bool setup(const std::string& remote_url, const std::string& method);

    // Check if sync is needed (based on interval)
    bool should_auto_sync() const;

    // Auto-sync if enabled and interval passed
    bool auto_sync_if_needed();

private:
    // Sync method implementations
    bool pull_git();
    bool push_git();

    bool pull_rsync();
    bool push_rsync();

    bool pull_file();
    bool push_file();

    bool pull_http();

    // Helpers
    bool validate_remote_url(const std::string& url, const std::string& method) const;
    std::string get_sync_cache_dir() const;
    bool ensure_sync_cache_dir() const;

    // Git helpers
    bool git_clone_or_pull(const std::string& repo_url, const std::string& target_dir);
    bool git_commit_and_push(const std::string& repo_dir, const std::string& message);

    // File copy helpers
    bool copy_config_files(const std::string& source_dir, const std::string& dest_dir);
    bool is_git_repo(const std::string& dir) const;
};

} // namespace aliases
