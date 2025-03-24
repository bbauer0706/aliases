#!/bin/bash
##############################################################################
#                              NPM SHORTCUTS                                 #
##############################################################################

# Basic NPM commands
alias dev='n dev'
# Define mappings of shorthand commands to actual npm commands
declare -A npm_commands=(
  [gen]="run gen"
  [b]="run build"
  [bs]="run build:static"
  [dev]="run dev"
  [start]="start"
  [i]="install"
  [t]="test"
)

# Main function for npm operations
n() {
  # Display help if no arguments are provided
  if [[ $# -eq 0 ]]; then
    echo "npm command shortcuts:"
    for cmd in "${!npm_commands[@]}"; do
      echo "  $cmd => npm ${npm_commands[$cmd]}"
    done
    echo "For any other npm script, use: n <script-name>"
    return
  fi
  
  local cmd="$1"
  shift # Remove the first argument
  
  # Check if it's a known shorthand command
  if [[ -n "${npm_commands[$cmd]}" ]]; then
    # For 'dev' command, add repository command if available
    if [[ "$cmd" == "dev" ]]; then
      # Try to get repository command name and run it first
      { 
        REPO_CMD=$(basename $(git rev-parse --show-toplevel 2>/dev/null)) && eval $REPO_CMD || echo ""
      } 
      npm ${npm_commands[$cmd]} "$@"
    else
      # Run the mapped npm command
      npm ${npm_commands[$cmd]} "$@"
    fi
  else
    # For any other commands, just pass to npm run
    npm run "$cmd" "$@"
  fi
}

# Tab completion for the 'n' function
_n_completion() {
  local cur=${COMP_WORDS[COMP_CWORD]}
  local npm_scripts=()
  
  # Add the known shorthand commands
  for cmd in "${!npm_commands[@]}"; do
    npm_scripts+=("$cmd")
  done
  
  # Try to extract available scripts from package.json if it exists
  if [ -f "package.json" ]; then
    # This uses a simplified approach to extract script names from package.json
    # For better results consider using jq if available
    local package_scripts=$(grep -o '"[a-zA-Z0-9:_-]*":' package.json | tr -d '":')
    for script in $package_scripts; do
      npm_scripts+=("$script")
    done
  fi
  
  # Set completion reply
  COMPREPLY=($(compgen -W "${npm_scripts[*]}" -- "$cur"))
  return 0
}

complete -F _n_completion n