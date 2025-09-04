#!/bin/bash
##############################################################################
#                                                                            #
#                          BASIC BASH ALIASES                                #
#                                                                            #
##############################################################################

# Directory listing
alias la='ls -alh'
alias ..='cd ..'
alias ...='cd ../..'
alias ~='cd ~'
alias clera='clear'
alias clare='clear'

# Kill process on port
kp() {
  fuser -k $1/tcp
}

# typescript
alias tsc='npx tsc --noEmit'

# Git aliases
alias git-reset-last='git reset --soft HEAD~1'

# Show current project environment variables
show_env() {
    # Get the directory where this script is located
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local cli_path="$script_dir/../build/aliases-cli"
    
    if [ ! -f "$cli_path" ]; then
        echo "Error: aliases-cli not found at $cli_path" >&2
        return 1
    fi
    
    "$cli_path" env --show
}
