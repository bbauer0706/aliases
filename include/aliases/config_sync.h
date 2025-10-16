#pragma once

#include "common.h"
#include <string>

namespace aliases {

/**
 * Configuration sync manager
 *
 * Handles syncing configuration from remote HTTP sources
 * Simple fetch-based approach for read-only config distribution
 */
class ConfigSync {
public:
    ConfigSync() = default;
    ~ConfigSync() = default;

    // Sync operations
    bool pull();   // Fetch config files from remote URLs
    bool push();   // Not supported (read-only sync)
    bool status(); // Show sync status

    // Setup sync URLs
    bool setup(const std::string& config_url, const std::string& todo_url = "");

    // Check if sync is needed (based on interval)
    bool should_auto_sync() const;

    // Auto-sync if enabled and interval passed
    bool auto_sync_if_needed();
};

} // namespace aliases
