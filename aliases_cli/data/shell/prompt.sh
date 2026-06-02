#!/usr/bin/env bash
# Shell integration for aliases-cli – custom PS1 prompt formatting.
# Source this file from ~/.bash_aliases (done automatically by aliases-cli setup).
#
# After sourcing, opt in by calling:
#   aliases_setup_prompt
#
# Or set ALIASES_AUTO_SETUP_PROMPT=1 before sourcing to enable automatically.
#
# Configuration (in ~/.config/aliases-cli/config.json):
#   {
#     "prompt": {
#       "enabled": true,
#       "user_host_color": "bold_green",
#       "default_path_color": "bold_blue",
#       "path_replacements": [
#         { "env_var": "INSTROOT", "label": "INSTROOT", "color": "bold_yellow" }
#       ]
#     }
#   }

# ---------------------------------------------------------------------------
# _aliases_prompt_pwd – formatted CWD for PS1
# ---------------------------------------------------------------------------
_aliases_prompt_pwd() {
    aliases-cli pwd --ps1 2>/dev/null || printf '%s' "$PWD"
}

# ---------------------------------------------------------------------------
# aliases_setup_prompt – install the custom PS1
# ---------------------------------------------------------------------------
aliases_setup_prompt() {
    # Respect prompt.enabled config key
    local enabled
    enabled=$(aliases-cli config get prompt.enabled 2>/dev/null)
    if [[ "$enabled" == "false" ]]; then
        return
    fi

    local uh_color reset
    uh_color=$(aliases-cli pwd --user-host-color --ps1 2>/dev/null)
    reset=$'\001\033[0m\002'

    # Build PS1: user@host:path$
    PS1="${uh_color}\\u@\\h${reset}:\$(_aliases_prompt_pwd)\\$ "
    export PS1
}

# Auto-enable if requested
if [[ "${ALIASES_AUTO_SETUP_PROMPT:-0}" == "1" ]]; then
    aliases_setup_prompt
fi
