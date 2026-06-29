#!/usr/bin/env bash
# Shell integration for aliases – custom PS1 prompt formatting.
# Source this file from ~/.bash_aliases (done automatically by aliases setup).
#
# After sourcing, opt in by calling:
#   aliases_setup_prompt
#
# Or set ALIASES_AUTO_SETUP_PROMPT=1 before sourcing to enable automatically.
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
# Prompt cache – aliases is only invoked when the directory changes.
# On every other Enter press, PS1 reads the cached string at zero cost.
# ---------------------------------------------------------------------------
_ALIASES_PROMPT_CACHE=""
_ALIASES_PROMPT_CACHE_DIR=""

_aliases_update_prompt_cache() {
    if [[ "$PWD" != "$_ALIASES_PROMPT_CACHE_DIR" ]]; then
        _ALIASES_PROMPT_CACHE="$(aliases pwd --full-prompt --ps1 2>/dev/null)" \
            || _ALIASES_PROMPT_CACHE="${USER}@${HOSTNAME}:${PWD}"
        _ALIASES_PROMPT_CACHE_DIR="$PWD"
    fi
}

# ---------------------------------------------------------------------------
# aliases_setup_prompt – install the custom PS1
# ---------------------------------------------------------------------------
aliases_setup_prompt() {
    # Respect prompt.enabled config key
    local enabled
    enabled=$(aliases config get prompt.enabled 2>/dev/null)
    if [[ "$enabled" == "false" ]]; then
        return
    fi

    # Populate cache immediately so the first prompt is correct.
    _aliases_update_prompt_cache

    # Hook into PROMPT_COMMAND so the cache refreshes only on cd.
    # Preserve any existing PROMPT_COMMAND entries.
    PROMPT_COMMAND="_aliases_update_prompt_cache${PROMPT_COMMAND:+; $PROMPT_COMMAND}"

    # PS1 just reads the pre-built cached string – no subprocess, no Python.
    PS1="\${_ALIASES_PROMPT_CACHE}\\$ "
    export PS1
}

# Auto-enable if requested
if [[ "${ALIASES_AUTO_SETUP_PROMPT:-0}" == "1" ]]; then
    aliases_setup_prompt
fi
