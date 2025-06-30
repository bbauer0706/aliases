#!/bin/bash
##############################################################################
#                         VS CODE DEBUG TERMINAL SUPPORT                     #
##############################################################################

# Function to detect and handle VS Code debug terminals
setup_vscode_debug_terminal() {
    # Check if this is a VS Code terminal
    if [[ -n "$VSCODE_PID" || -n "$TERM_PROGRAM" ]] && [[ "$TERM_PROGRAM" == "vscode" ]]; then
        # Check if we're in a workspace directory
        if [[ "$(pwd)" == "$HOME/workspaces"/* ]]; then
            # Get the current project name
            local current_project_name
            local current_dir=$(pwd)
            local workspace_dir="$HOME/workspaces"
            
            if [[ "$current_dir" == "$workspace_dir"/* ]]; then
                local remaining_path="${current_dir#$workspace_dir/}"
                current_project_name="${remaining_path%%/*}"
            fi
            
            # If PROJECT_NAME is not set or different from current project
            if [[ -z "$PROJECT_NAME" || "$PROJECT_NAME" != "$current_project_name" ]]; then
                echo -e "\033[0;36m[VS Code Debug Terminal]\033[0m Setting up environment for project: $current_project_name"
                
                # Force reload of environment
                unset PROJECT_ENV_LOADED
                
                # Run project environment setup if available
                if command -v project_env >/dev/null 2>&1; then
                    project_env -p 8000
                    echo -e "\033[0;32m[SUCCESS]\033[0m Environment ready for debugging"
                else
                    echo -e "\033[0;33m[WARNING]\033[0m project_env not available, using defaults"
                    # Set basic defaults
                    export PROJECT_NAME="$current_project_name"
                    export PROFILE=dev
                    export GQLHOST=$(hostname)
                    export WEBPORT=8000
                    export GQLPORT=8001
                    export SBPORT=8002
                    export NDEBUGPORT=8003
                fi
            fi
        fi
    fi
}

# Run setup immediately when this file is sourced
setup_vscode_debug_terminal

# Also create an alias for manual triggering
alias setup_debug='setup_vscode_debug_terminal'
alias debug_env='setup_vscode_debug_terminal'

# Test aliases for debugging project environment
alias test_env='/home/bbauer/workspaces/aliases/test-env.sh'
alias env_test='/home/bbauer/workspaces/aliases/test-env.sh'
