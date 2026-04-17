#include "aliases/commands/secrets_cmd.h"
#include "aliases/config.h"
#include "aliases/common.h"

#include <openssl/crypto.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <termios.h>
#include <unistd.h>

namespace aliases::commands {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

SecretsCmd::SecretsCmd(std::shared_ptr<ProjectMapper> mapper)
    : mapper_(std::move(mapper)) {}

// ---------------------------------------------------------------------------
// Dispatch
// ---------------------------------------------------------------------------

int SecretsCmd::execute(const StringVector& args) {
    if (args.empty() || args[0] == "--help" || args[0] == "-h") {
        show_help();
        return args.empty() ? 2 : 0;
    }

    const auto& sub = args[0];
    const StringVector rest(args.begin() + 1, args.end());

    if (sub == "set")                          return cmd_set(rest);
    if (sub == "get")                          return cmd_get(rest);
    if (sub == "list")                         return cmd_list();
    if (sub == "delete" || sub == "remove"
                        || sub == "rm")        return cmd_delete(rest);
    if (sub == "load"   || sub == "export")    return cmd_load(rest);
    if (sub == "rotate-master")                return cmd_rotate_master();

    std::cerr << "Error: unknown subcommand '" << sub << "'\n\n";
    show_help();
    return 2;
}

// ---------------------------------------------------------------------------
// Subcommand implementations
// ---------------------------------------------------------------------------

int SecretsCmd::cmd_set(const StringVector& args) {
    if (args.empty()) {
        std::cerr << "Usage: aliases secrets set <name> [value]\n";
        return 2;
    }

    const std::string& name = args[0];

    // Validate name: only alphanumeric, underscore, hyphen.
    for (char c : name) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
            std::cerr << "Error: secret name '" << name
                      << "' contains invalid characters (use [A-Za-z0-9_-])\n";
            return 2;
        }
    }

    std::string value;
    if (args.size() > 1) {
        value = args[1];
    } else {
        // Prompt securely so the value never appears in shell history.
        value = prompt_password("Value for '" + name + "': ");
    }

    std::string password = get_master_password();
    if (password.empty()) {
        std::cerr << "Error: master password is required\n";
        return 1;
    }

    auto store = open_store();
    auto unlock = store.unlock(password);
    OPENSSL_cleanse(password.data(), password.size());

    if (!unlock) {
        std::cerr << "Error: " << unlock.error_message << "\n";
        return 1;
    }

    store.set(name, value);
    OPENSSL_cleanse(value.data(), value.size());

    auto saved = store.save();
    if (!saved) {
        std::cerr << "Error: " << saved.error_message << "\n";
        return 1;
    }

    std::cout << Colors::SUCCESS << "Secret '" << name << "' stored."
              << Colors::RESET << "\n";
    return 0;
}

int SecretsCmd::cmd_get(const StringVector& args) {
    if (args.empty()) {
        std::cerr << "Usage: aliases secrets get <name>\n";
        return 2;
    }

    const std::string& name = args[0];

    std::string password = get_master_password();
    if (password.empty()) {
        std::cerr << "Error: master password is required\n";
        return 1;
    }

    auto store = open_store();
    if (!store.store_exists()) {
        OPENSSL_cleanse(password.data(), password.size());
        std::cerr << "Error: no secrets store found — use 'aliases secrets set' first\n";
        return 1;
    }

    auto unlock = store.unlock(password);
    OPENSSL_cleanse(password.data(), password.size());

    if (!unlock) {
        std::cerr << "Error: " << unlock.error_message << "\n";
        return 1;
    }

    auto result = store.get(name);
    if (!result) {
        std::cerr << "Error: " << result.error_message << "\n";
        return 1;
    }

    std::cout << result.value << "\n";
    return 0;
}

int SecretsCmd::cmd_list() {
    std::string password = get_master_password();
    if (password.empty()) {
        std::cerr << "Error: master password is required\n";
        return 1;
    }

    auto store = open_store();
    if (!store.store_exists()) {
        OPENSSL_cleanse(password.data(), password.size());
        std::cout << "(no secrets stored)\n";
        return 0;
    }

    auto unlock = store.unlock(password);
    OPENSSL_cleanse(password.data(), password.size());

    if (!unlock) {
        std::cerr << "Error: " << unlock.error_message << "\n";
        return 1;
    }

    const auto names = store.list_names();
    if (names.empty()) {
        std::cout << "(no secrets stored)\n";
        return 0;
    }

    for (const auto& n : names) {
        std::cout << n << "\n";
    }
    return 0;
}

int SecretsCmd::cmd_delete(const StringVector& args) {
    if (args.empty()) {
        std::cerr << "Usage: aliases secrets delete <name>\n";
        return 2;
    }

    const std::string& name = args[0];

    // Confirm if destructive actions are enabled.
    if (Config::instance().get_confirm_destructive_actions()) {
        std::cout << "Delete secret '" << name << "'? [y/N] ";
        std::string answer;
        std::getline(std::cin, answer);
        if (answer != "y" && answer != "Y") {
            std::cout << "Aborted.\n";
            return 0;
        }
    }

    std::string password = get_master_password();
    if (password.empty()) {
        std::cerr << "Error: master password is required\n";
        return 1;
    }

    auto store = open_store();
    if (!store.store_exists()) {
        OPENSSL_cleanse(password.data(), password.size());
        std::cerr << "Error: no secrets store found\n";
        return 1;
    }

    auto unlock = store.unlock(password);
    OPENSSL_cleanse(password.data(), password.size());

    if (!unlock) {
        std::cerr << "Error: " << unlock.error_message << "\n";
        return 1;
    }

    auto removed = store.remove(name);
    if (!removed) {
        std::cerr << "Error: " << removed.error_message << "\n";
        return 1;
    }

    auto saved = store.save();
    if (!saved) {
        std::cerr << "Error: " << saved.error_message << "\n";
        return 1;
    }

    std::cout << Colors::SUCCESS << "Secret '" << name << "' deleted."
              << Colors::RESET << "\n";
    return 0;
}

int SecretsCmd::cmd_load(const StringVector& args) {
    std::string password = get_master_password();
    if (password.empty()) {
        std::cerr << "Error: master password is required\n";
        return 1;
    }

    auto store = open_store();
    if (!store.store_exists()) {
        OPENSSL_cleanse(password.data(), password.size());
        // Empty store — output nothing; not an error for scripted use.
        return 0;
    }

    auto unlock = store.unlock(password);
    OPENSSL_cleanse(password.data(), password.size());

    if (!unlock) {
        std::cerr << "Error: " << unlock.error_message << "\n";
        return 1;
    }

    if (args.empty()) {
        // Export all secrets.
        const auto all = store.get_all();
        // Sort by key for deterministic output.
        StringVector keys;
        keys.reserve(all.size());
        for (const auto& [k, _] : all) keys.push_back(k);
        std::sort(keys.begin(), keys.end());

        for (const auto& k : keys) {
            std::cout << "export " << k << "=" << shell_single_quote(all.at(k)) << "\n";
        }
    } else {
        // Export only the named secrets.
        bool had_error = false;
        for (const auto& name : args) {
            auto r = store.get(name);
            if (!r) {
                std::cerr << "Warning: " << r.error_message << "\n";
                had_error = true;
                continue;
            }
            std::cout << "export " << name << "=" << shell_single_quote(r.value) << "\n";
        }
        if (had_error) return 1;
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

int SecretsCmd::cmd_rotate_master() {
    if (!open_store().store_exists()) {
        std::cerr << "Error: no secrets store found — nothing to rotate\n";
        return 1;
    }

    // Unlock with the current password.
    std::string old_password = get_master_password();
    if (old_password.empty()) {
        std::cerr << "Error: current master password is required\n";
        return 1;
    }

    auto store = open_store();
    auto unlock_result = store.unlock(old_password);
    OPENSSL_cleanse(old_password.data(), old_password.size());

    if (!unlock_result) {
        std::cerr << "Error: " << unlock_result.error_message << "\n";
        return 1;
    }

    // Prompt for new password twice to confirm.
    const std::string new_pw1 = prompt_password("New master password: ");
    if (new_pw1.empty()) {
        std::cerr << "Error: new password must not be empty\n";
        return 1;
    }
    const std::string new_pw2 = prompt_password("Confirm new master password: ");

    if (new_pw1 != new_pw2) {
        OPENSSL_cleanse(const_cast<char*>(new_pw1.data()), new_pw1.size());
        OPENSSL_cleanse(const_cast<char*>(new_pw2.data()), new_pw2.size());
        std::cerr << "Error: passwords do not match\n";
        return 1;
    }

    // Snapshot all secrets from the unlocked store.
    const aliases::StringMap all = store.get_all();

    // Remove the existing encrypted file so the new store starts fresh
    // (unlock() would otherwise try to decrypt it with the wrong password).
    const std::string store_path = Config::instance().get_secrets_store_path();
    if (std::remove(store_path.c_str()) != 0) {
        std::cerr << "Error: could not remove old store file to rotate\n";
        return 1;
    }

    // Create a fresh store instance, unlock with new password, re-populate, save.
    auto new_store = open_store();
    auto new_unlock = new_store.unlock(new_pw1);
    OPENSSL_cleanse(const_cast<char*>(new_pw1.data()), new_pw1.size());
    OPENSSL_cleanse(const_cast<char*>(new_pw2.data()), new_pw2.size());

    if (!new_unlock) {
        std::cerr << "Error: failed to initialise new store: "
                  << new_unlock.error_message << "\n";
        return 1;
    }

    for (const auto& [k, v] : all) {
        new_store.set(k, v);
    }

    auto saved = new_store.save();
    if (!saved) {
        std::cerr << "Error: " << saved.error_message << "\n";
        return 1;
    }

    std::cout << Colors::SUCCESS << "Master password rotated. "
              << all.size() << " secret(s) re-encrypted."
              << Colors::RESET << "\n";
    return 0;
}

void SecretsCmd::show_help() const {
    std::cout <<
        "Usage: aliases secrets <subcommand> [args]\n"
        "\n"
        "Manage encrypted environment variable secrets.\n"
        "\n"
        "Subcommands:\n"
        "  set           <name> [value]  Store a secret (prompts securely if value omitted)\n"
        "  get           <name>          Print the decrypted value\n"
        "  list                          List all secret names\n"
        "  delete        <name>          Remove a secret\n"
        "  load          [name...]       Output 'export NAME=VALUE' lines for eval\n"
        "  rotate-master                 Re-encrypt the store with a new master password\n"
        "\n"
        "Master password:\n"
        "  Set $ALIASES_MASTER_PASSWORD (or the configured env var) to avoid\n"
        "  interactive prompts.  If unset, you will be prompted on the terminal.\n"
        "\n"
        "Examples:\n"
        "  aliases secrets set MY_API_KEY           # prompts for value\n"
        "  aliases secrets set DB_PASS mysecret\n"
        "  aliases secrets list\n"
        "  aliases secrets get MY_API_KEY\n"
        "  aliases secrets delete MY_API_KEY\n"
        "  eval \"$(aliases secrets load)\"           # export all into current shell\n"
        "  eval \"$(aliases secrets load MY_API_KEY)\" # export one secret\n"
        "  aliases secrets rotate-master            # change master password\n";
}

SecretsStore SecretsCmd::open_store() const {
    const auto& cfg = Config::instance();
    const std::string path = cfg.get_secrets_store_path();
    const int iterations   = cfg.get_secrets_kdf_iterations();
    return SecretsStore(path, iterations);
}

std::string SecretsCmd::get_master_password() const {
    const auto& cfg = Config::instance();
    const std::string env_var = cfg.get_secrets_password_env_var();

    // Prefer env var for non-interactive / scripted use.
    const char* env_val = std::getenv(env_var.c_str());
    if (env_val && env_val[0] != '\0') {
        return std::string(env_val);
    }

    return prompt_password("Secrets master password: ");
}

std::string SecretsCmd::prompt_password(const std::string& prompt) {
    // Try /dev/tty so password prompts work even when stdout is redirected
    // (e.g., inside eval "$(aliases secrets load)").
    FILE* tty = fopen("/dev/tty", "r+");
    if (tty) {
        const int tty_fd = fileno(tty);
        struct termios old_t{};
        tcgetattr(tty_fd, &old_t);
        struct termios new_t = old_t;
        new_t.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        new_t.c_lflag |=  static_cast<tcflag_t>(ECHONL);
        tcsetattr(tty_fd, TCSANOW, &new_t);

        fprintf(tty, "%s", prompt.c_str());
        fflush(tty);

        std::string password;
        for (int ch; (ch = fgetc(tty)) != '\n' && ch != EOF; ) {
            password += static_cast<char>(ch);
        }

        tcsetattr(tty_fd, TCSANOW, &old_t);
        fclose(tty);
        return password;
    }

    // Fallback: write prompt to stderr, read from stdin with echo off.
    std::cerr << prompt;
    struct termios old_t{};
    tcgetattr(STDIN_FILENO, &old_t);
    struct termios new_t = old_t;
    new_t.c_lflag &= ~static_cast<tcflag_t>(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t);

    std::string password;
    std::getline(std::cin, password);

    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
    std::cerr << "\n";
    return password;
}

std::string SecretsCmd::shell_single_quote(const std::string& value) {
    // POSIX single-quote escape: replace each ' with '\''
    std::string result = "'";
    for (const char c : value) {
        if (c == '\'') {
            result += "'\\''";
        } else {
            result += c;
        }
    }
    result += "'";
    return result;
}

} // namespace aliases::commands
