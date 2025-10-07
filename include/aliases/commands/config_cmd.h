#pragma once

#include "../common.h"
#include "../project_mapper.h"

namespace aliases::commands {

/**
 * Config command - manage aliases-cli configuration
 *
 * Subcommands:
 * - get <key>          - Get a config value
 * - set <key> <value>  - Set a config value
 * - list               - List all configuration settings
 * - reset              - Reset config to defaults
 * - edit               - Open config file in editor
 * - path               - Show config file path
 */
class ConfigCmd {
public:
    explicit ConfigCmd(std::shared_ptr<ProjectMapper> mapper);
    ~ConfigCmd() = default;

    // Main command entry point
    int execute(const StringVector& args);

private:
    std::shared_ptr<ProjectMapper> project_mapper_;

    // Subcommands
    void show_help() const;
    int cmd_get(const StringVector& args);
    int cmd_set(const StringVector& args);
    int cmd_list(const StringVector& args);
    int cmd_reset(const StringVector& args);
    int cmd_edit(const StringVector& args);
    int cmd_path(const StringVector& args);
};

} // namespace aliases::commands
