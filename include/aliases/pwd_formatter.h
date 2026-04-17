#pragma once

#include <string>
#include <vector>

namespace aliases {

/**
 * A rule that maps a path prefix to a short label.
 *
 * Exactly one of the two matching modes must be set (XOR):
 *   - env_var: prefix is the runtime-expanded value of this env variable
 *   - path:    prefix is a literal path (may contain ~ for $HOME)
 *
 * Rules with both or neither set are ignored. The first matching rule wins.
 */
struct PromptPathReplacement {
    std::string env_var;  // e.g. "INSTROOT"  — expand at runtime
    std::string path;     // e.g. "/opt/company/platform"  — literal prefix
    std::string label;    // e.g. "INSTROOT" or "PLATFORM"  (shown in prompt)
    std::string color;    // ANSI color name, e.g. "bold_cyan"
};

/**
 * Formats the current working directory for use in a shell prompt.
 *
 * Path replacement rules are applied in order; the first matching rule wins.
 * If no rule matches, the path is shortened with ~ for $HOME.
 */
class PwdFormatter {
public:
    /**
     * Format `path` using the supplied replacement rules.
     *
     * @param path               Absolute path to format (typically $PWD / getcwd()).
     * @param replacements       Ordered list of path-replacement rules from config.
     * @param colors             When true, emit ANSI color codes around the label.
     * @param ps1_mode           When true, wrap ANSI codes in \001...\002 so bash
     *                           does not count them toward the line length.
     * @param default_path_color Color name (e.g. "bold_blue") applied to the path
     *                           when no replacement rule matches. Empty = no color.
     */
    static std::string format(
        const std::string& path,
        const std::vector<PromptPathReplacement>& replacements,
        bool colors,
        bool ps1_mode,
        const std::string& default_path_color = ""
    );

    /** Return the ANSI escape sequence for the given color name, or "". */
    static std::string ansi_code(const std::string& color_name);

    /** Return the ANSI reset sequence. */
    static std::string ansi_reset();

    /** Wrap an ANSI sequence in PS1 non-printing delimiters (\001 / \002). */
    static std::string ps1_wrap(const std::string& ansi);
};

} // namespace aliases
