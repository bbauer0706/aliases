#!/usr/bin/env bash
# Shell integration for aliases-cli – project environment setup.
# Source this file from ~/.bash_aliases (done automatically by aliases-cli setup).
#
# Provides:
#   project_env [OPTIONS]   – discover current project and export env vars
#   show_env                – display current project env vars
#   refresh_project_env     – re-run project_env and show result
#
# Aliases: fix_env, fix_project, project_fix → refresh_project_env

# ---------------------------------------------------------------------------
# project_env
# ---------------------------------------------------------------------------
project_env() {
    local stdout stderr exit_code tmp_stderr
    tmp_stderr=$(mktemp)

    # Single invocation: capture stdout (exports) and let stderr flow through
    stdout=$(aliases-cli env "$@" 2>"$tmp_stderr")
    exit_code=$?
    stderr=$(cat "$tmp_stderr")
    rm -f "$tmp_stderr"

    if [[ $exit_code -ne 0 ]]; then
        echo "Error: aliases-cli env failed (exit $exit_code)" >&2
        [[ -n "$stderr" ]] && echo "$stderr" >&2
        return $exit_code
    fi

    eval "$stdout"
    # Print the status message that the CLI wrote to stderr
    [[ -n "$stderr" ]] && echo "$stderr" >&2
}

# ---------------------------------------------------------------------------
# show_env
# ---------------------------------------------------------------------------
show_env() {
    aliases-cli env --show
}

# ---------------------------------------------------------------------------
# refresh_project_env
# ---------------------------------------------------------------------------
refresh_project_env() {
    echo "Refreshing project environment…"
    project_env "$@" && show_env
}

alias fix_env='refresh_project_env'
alias fix_project='refresh_project_env'
alias project_fix='refresh_project_env'

# ---------------------------------------------------------------------------
# auto_setup_new_terminal (opt-in)
# ---------------------------------------------------------------------------
# Set ALIASES_AUTO_SETUP_ENV=1 before sourcing this file to auto-run
# project_env when opening a new terminal inside a workspace directory.
if [[ "${ALIASES_AUTO_SETUP_ENV:-0}" == "1" ]]; then
    _auto_setup_new_terminal() {
        [[ -n "$PROJECT_NAME" ]] && return
        # Run project_env; it exits cleanly if CWD isn't inside a workspace dir.
        project_env -e "${DEFAULT_ENV:-dev}" 2>/dev/null || true
    }
    _auto_setup_new_terminal
    unset -f _auto_setup_new_terminal
fi
