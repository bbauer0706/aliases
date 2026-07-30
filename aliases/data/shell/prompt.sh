#!/usr/bin/env bash
# Shell integration for aliases – custom PS1 prompt formatting.
# Source this file from ~/.bash_aliases (done automatically by aliases setup).
#
# The prompt installs itself on source. To disable it:
#   ALIASES_NO_PROMPT=1            – per shell, no subprocess (set it before sourcing)
#   aliases config set prompt.enabled false   – globally
#
# PS1 is deliberately NOT exported and holds no variable references: child
# processes that snapshot the environment must not inherit an unexpandable
# ${...} they know nothing about.
#
# Configuration (in ~/.config/aliases/config.json):
#   {
#     "prompt": {
#       "enabled": true,
#       "user_host_color": "bold_green",
#       "default_path_color": "bold_blue",
#       "host_replacements": [{"hostname": "host123, "label": "prod"}],
#       "user_replacements": [{"username": "user123", "label": "user"}],
#       "path_replacements": [
#         { "env_var": "INSTROOT", "label": "INSTROOT", "color": "bold_yellow" }
#       ]
#     }
#   }

# ---------------------------------------------------------------------------
# aliases is only invoked when the directory changes. On every other Enter
# press PROMPT_COMMAND sees an unchanged $PWD and leaves PS1 alone.
# ---------------------------------------------------------------------------
_ALIASES_PROMPT_DIR=""

_aliases_update_prompt() {
    if [[ "$PWD" != "$_ALIASES_PROMPT_DIR" ]]; then
        PS1="$(aliases pwd --full-prompt --ps1 2>/dev/null)"
        [[ -n "$PS1" ]] || PS1="${USER}@${HOSTNAME}:${PWD}"
        PS1+='\$ '
        _ALIASES_PROMPT_DIR="$PWD"
    fi
}

# ---------------------------------------------------------------------------
# _aliases_setup_prompt – install the custom PS1
# ---------------------------------------------------------------------------
_aliases_setup_prompt() {
    # Checked first: an opt-out must not cost an `aliases` subprocess.
    [[ -n "$ALIASES_NO_PROMPT" ]] && return

    # Respect prompt.enabled config key. Older CLI versions print Python's
    # "False", newer ones "false" – ,, accepts both.
    local enabled
    enabled=$(aliases config get prompt.enabled 2>/dev/null)
    [[ "${enabled,,}" == "false" ]] && return

    # Populate immediately so the first prompt is correct.
    _aliases_update_prompt

    # Refresh only on cd, preserving any existing PROMPT_COMMAND entries.
    PROMPT_COMMAND="_aliases_update_prompt${PROMPT_COMMAND:+; $PROMPT_COMMAND}"
}

_aliases_setup_prompt

unset -f _aliases_setup_prompt
