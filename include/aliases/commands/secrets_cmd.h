#pragma once

#include "aliases/common.h"
#include "aliases/project_mapper.h"
#include "aliases/secrets_store.h"
#include <memory>
#include <string>

namespace aliases::commands {

/**
 * `aliases secrets` — encrypted environment variable / secret manager.
 *
 * Subcommands:
 *   set             <name> [value]   Store a secret (prompts securely if value omitted)
 *   get             <name>           Print the decrypted value of a secret
 *   list                             List all stored secret names
 *   delete          <name>           Remove a secret
 *   load            [name...]        Output "export NAME=VALUE" lines suitable for eval
 *   rotate-master                    Re-encrypt the store with a new master password
 *
 * The secrets store is encrypted with AES-256-GCM.  The master password is
 * read from the env var specified by config key secrets.password_env_var
 * (default: ALIASES_MASTER_PASSWORD).  If that variable is unset, the user
 * is prompted interactively with echo disabled.
 */
class SecretsCmd {
public:
    explicit SecretsCmd(std::shared_ptr<ProjectMapper> mapper);
    int execute(const StringVector& args);

private:
    std::shared_ptr<ProjectMapper> mapper_;

    int cmd_set(const StringVector& args);
    int cmd_get(const StringVector& args);
    int cmd_list();
    int cmd_delete(const StringVector& args);
    int cmd_load(const StringVector& args);
    int cmd_rotate_master();

    void show_help() const;

    // Opens a SecretsStore configured from Config.
    SecretsStore open_store() const;

    // Returns the master password (env var → interactive prompt).
    std::string get_master_password() const;

    // Prompts the user on /dev/tty (or stderr) with echo disabled.
    static std::string prompt_password(const std::string& prompt);

    // Wraps a value in POSIX single-quotes with internal ' escaped.
    static std::string shell_single_quote(const std::string& value);
};

} // namespace aliases::commands
