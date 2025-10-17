#!/bin/bash
##############################################################################
#                                                                            #
#                    BASH INTEGRATION FOR PROJECT ENV                        #
#                                                                            #
##############################################################################

# Project Environment Setup
# Function to set up project environment variables using the C++ CLI tool
project_env() {
    # Get the directory where this script is located
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local cli_path="$script_dir/../build/aliases-cli"

    # Fallback to distributed binary if build doesn't exist
    if [ ! -f "$cli_path" ]; then
        cli_path="$script_dir/../aliases-cli"
    fi

    # Check if the CLI tool exists
    if [ ! -f "$cli_path" ]; then
        echo "Error: aliases-cli not found at $cli_path" >&2
        echo "Please run 'make' to build the CLI tool first." >&2
        return 1
    fi

    # Call the C++ tool and capture its output
    local env_output
    env_output=$("$cli_path" env "$@" 2>/dev/null)
    local exit_code=$?

    if [ $exit_code -eq 0 ]; then
        # Eval the output to set environment variables in the current shell
        eval "$env_output"
        # Show success message (the C++ tool outputs this to stderr)
        "$cli_path" env "$@" >/dev/null
    else
        echo "Error: Failed to set up project environment" >&2
        return $exit_code
    fi
}

# Function to display current environment variables
show_env() {
    # Get the directory where this script is located
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local cli_path="$script_dir/../build/aliases-cli"

    # Fallback to distributed binary if build doesn't exist
    if [ ! -f "$cli_path" ]; then
        cli_path="$script_dir/../aliases-cli"
    fi

    # Check if the CLI tool exists
    if [ ! -f "$cli_path" ]; then
        echo "Error: aliases-cli not found" >&2
        return 1
    fi

    # Use the C++ tool to show environment variables
    "$cli_path" env --show
}

# Legacy compatibility functions (call C++ version internally)
refresh_project_env() {
    echo "Refreshing project environment for current directory..."
    if project_env -p "${WEBPORT:-8000}"; then
        show_env
    fi
}

show_env_vars() {
    show_env
}

# Convenient aliases for project environment management
alias fix_env='refresh_project_env'
alias fix_project='refresh_project_env'
alias project_fix='refresh_project_env'

# Auto-setup for new terminals (only in workspace directories)
auto_setup_new_terminal() {
    if [[ "$(pwd)" == "$HOME/workspaces"/* ]]; then
        if [[ -z "$PROJECT_NAME" ]]; then
            project_env -p "${WEBPORT:-8000}" -t plain 2>/dev/null || true
        fi
    fi
}
