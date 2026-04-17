#!/bin/bash
##############################################################################
#                                                                            #
#                  BASH INTEGRATION FOR PROMPT FORMATTING                    #
#                                                                            #
##############################################################################
#
# Provides a customized PS1 that replaces long path prefixes (such as $INSTROOT)
# with short, optionally-coloured labels configured in aliases-cli.
#
# Usage in ~/.bashrc:
#   source /path/to/bash_integration/prompt.sh
#
#   # Then opt in with:
#   aliases_setup_prompt
#
#   # Or set ALIASES_AUTO_SETUP_PROMPT=1 before sourcing to enable automatically:
#   export ALIASES_AUTO_SETUP_PROMPT=1
#   source /path/to/bash_integration/prompt.sh
#
# The resulting PS1 looks like:
#   user@host:INSTROOT/my/sub/dir$
# where INSTROOT is coloured according to your config (default: bold cyan).
#
# Configuration (in ~/.config/aliases-cli/config.json):
#   {
#     "prompt": {
#       "enabled": true,
#       "path_replacements": [
#         { "env_var": "INSTROOT", "label": "INSTROOT", "color": "bold_cyan" }
#       ]
#     }
#   }
#
# Multiple rules are supported and evaluated in order; the first match wins.

# ---------------------------------------------------------------------------
# Internal: locate the aliases-cli binary
# ---------------------------------------------------------------------------
_aliases_cli_find() {
    local script_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

    local cli="$script_dir/../build/aliases-cli"
    [ -f "$cli" ] && { echo "$cli"; return; }

    cli="$script_dir/../aliases-cli"
    [ -f "$cli" ] && { echo "$cli"; return; }

    # Fall back to whatever is on PATH
    command -v aliases-cli 2>/dev/null
}

_ALIASES_CLI_PATH="$(_aliases_cli_find)"

# ---------------------------------------------------------------------------
# _aliases_prompt_pwd
#   Outputs the formatted current directory for use in PS1.
#   Calls `aliases-cli pwd --ps1` which wraps ANSI codes in \001..\002 so
#   bash does not count them toward the line length.
# ---------------------------------------------------------------------------
_aliases_prompt_pwd() {
    if [ -n "$_ALIASES_CLI_PATH" ]; then
        "$_ALIASES_CLI_PATH" pwd --ps1 2>/dev/null || echo -n "$PWD"
    else
        echo -n "$PWD"
    fi
}

# ---------------------------------------------------------------------------
# aliases_setup_prompt
#   Installs a PS1 that uses _aliases_prompt_pwd for the directory portion.
#   The user@host color is read from aliases-cli config (prompt.user_host_color).
#   Color decisions are delegated to the CLI (which respects general.terminal_colors),
#   so no $TERM check is needed here.
#   Safe to call multiple times (idempotent).
# ---------------------------------------------------------------------------
aliases_setup_prompt() {
    local uh_color reset
    if [ -n "$_ALIASES_CLI_PATH" ]; then
        # CLI returns a ps1-wrapped ANSI code, or empty string if colors are
        # disabled in config (general.terminal_colors = false).
        uh_color=$("$_ALIASES_CLI_PATH" pwd --user-host-color --ps1 2>/dev/null)
        reset=$(printf '\001\033[0m\002')
    fi

    if [ -n "$uh_color" ]; then
        PS1="${uh_color}\u@\h${reset}:\$(_aliases_prompt_pwd)\$ "
    else
        PS1='\u@\h:$(_aliases_prompt_pwd)\$ '
    fi
    export PS1
}

# ---------------------------------------------------------------------------
# Auto-enable if the environment variable is set before sourcing
# ---------------------------------------------------------------------------
if [ "${ALIASES_AUTO_SETUP_PROMPT:-0}" = "1" ]; then
    aliases_setup_prompt
fi
