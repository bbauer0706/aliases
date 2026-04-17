#include "aliases/pwd_formatter.h"

#include <cstdlib>
#include <unordered_map>

namespace aliases {

// ---------------------------------------------------------------------------
// Colour lookup table
// ---------------------------------------------------------------------------

static const std::unordered_map<std::string, std::string> COLOR_MAP = {
    {"reset",        "\033[0m"},
    {"black",        "\033[0;30m"},
    {"red",          "\033[0;31m"},
    {"green",        "\033[0;32m"},
    {"yellow",       "\033[0;33m"},
    {"blue",         "\033[0;34m"},
    {"magenta",      "\033[0;35m"},
    {"cyan",         "\033[0;36m"},
    {"white",        "\033[0;37m"},
    {"bold_black",   "\033[1;30m"},
    {"bold_red",     "\033[1;31m"},
    {"bold_green",   "\033[1;32m"},
    {"bold_yellow",  "\033[1;33m"},
    {"bold_blue",    "\033[1;34m"},
    {"bold_magenta", "\033[1;35m"},
    {"bold_cyan",    "\033[1;36m"},
    {"bold_white",   "\033[1;37m"},
};

// ---------------------------------------------------------------------------
// PwdFormatter implementation
// ---------------------------------------------------------------------------

std::string PwdFormatter::ansi_code(const std::string& color_name) {
    auto it = COLOR_MAP.find(color_name);
    return it != COLOR_MAP.end() ? it->second : "";
}

std::string PwdFormatter::ansi_reset() {
    return "\033[0m";
}

std::string PwdFormatter::ps1_wrap(const std::string& ansi) {
    if (ansi.empty()) return "";
    // \001 == RL_PROMPT_START_IGNORE, \002 == RL_PROMPT_END_IGNORE
    return "\001" + ansi + "\002";
}

std::string PwdFormatter::format(
    const std::string& path,
    const std::vector<PromptPathReplacement>& replacements,
    bool colors,
    bool ps1_mode,
    const std::string& default_path_color)
{
    // Helper: resolve a rule to its expanded prefix string.
    // Returns "" for invalid rules (both or neither of env_var/path set).
    auto resolve_prefix = [](const PromptPathReplacement& rule) -> std::string {
        bool has_env  = !rule.env_var.empty();
        bool has_path = !rule.path.empty();

        // XOR: exactly one must be set.
        if (has_env == has_path) return "";

        std::string prefix;

        if (has_env) {
            const char* env_val = std::getenv(rule.env_var.c_str());
            if (!env_val || *env_val == '\0') return "";
            prefix = env_val;
        } else {
            prefix = rule.path;
            // Expand leading ~ to $HOME.
            if (!prefix.empty() && prefix[0] == '~') {
                const char* home = std::getenv("HOME");
                if (home && *home != '\0') {
                    prefix = std::string(home) + prefix.substr(1);
                }
            }
        }

        // Strip trailing slashes for clean matching.
        while (prefix.size() > 1 && prefix.back() == '/') prefix.pop_back();
        return prefix;
    };

    // Try each rule in order; first match wins.
    for (const auto& rule : replacements) {
        std::string prefix = resolve_prefix(rule);
        if (prefix.empty()) continue;
        if (path.rfind(prefix, 0) != 0) continue;  // no prefix match

        std::string suffix = path.substr(prefix.size());
        // suffix is either empty or starts with '/'.

        std::string label_text;
        std::string suffix_text;
        if (colors) {
            std::string code = ansi_code(rule.color);
            std::string rst  = ansi_reset();
            if (ps1_mode) {
                label_text = ps1_wrap(code) + rule.label + ps1_wrap(rst);
            } else {
                label_text = code + rule.label + rst;
            }
            // Apply default_path_color to the suffix, if configured.
            if (!suffix.empty() && !default_path_color.empty()) {
                std::string dcode = ansi_code(default_path_color);
                if (!dcode.empty()) {
                    if (ps1_mode) {
                        suffix_text = ps1_wrap(dcode) + suffix + ps1_wrap(rst);
                    } else {
                        suffix_text = dcode + suffix + rst;
                    }
                }
            }
        } else {
            label_text = rule.label;
        }

        return label_text + (suffix_text.empty() ? suffix : suffix_text);
    }

    // No rule matched – apply ~ substitution for $HOME, then optionally color.
    std::string result;
    const char* home_env = std::getenv("HOME");
    if (home_env && *home_env != '\0') {
        std::string home(home_env);
        while (home.size() > 1 && home.back() == '/') home.pop_back();
        if (path.rfind(home, 0) == 0) {
            result = "~" + path.substr(home.size());
        }
    }
    if (result.empty()) result = path;

    if (colors && !default_path_color.empty()) {
        std::string code = ansi_code(default_path_color);
        std::string rst  = ansi_reset();
        if (!code.empty()) {
            if (ps1_mode) {
                return ps1_wrap(code) + result + ps1_wrap(rst);
            } else {
                return code + result + rst;
            }
        }
    }
    return result;
}

} // namespace aliases
