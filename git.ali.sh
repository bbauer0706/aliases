#!/bin/bash
##############################################################################
#                               GIT SHORTCUTS                                #
##############################################################################

# Define mappings of shorthand commands to git commands
declare -A git_commands=(
  [s]="status"
  [a]="add ."
  [c]="commit -m"
  [ac]="add . && git commit -m"
  [p]="pull"
  [ps]="push"
  [psf]="push -f"
  [co]="checkout"
  [nb]="checkout -b"
  [nf]="checkout -b features/"
  [l]="log --oneline"
  [b]="branch"
  [cl]="clone"
  [m]="merge"
  [d]="diff"
  [r]="rebase"
)

g() {
  # Help parameters
  if [[ $# -eq 0 || "$1" == "-h" || "$1" == "--help" ]]; then
    echo "git command shortcuts:"
    for cmd in "${!git_commands[@]}"; do
      echo "  $cmd => git ${git_commands[$cmd]}"
    done
    return
  fi

  local cmd="$1"
  shift # Remove the first argument

  # Check if it's a known shorthand command
  if [[ -n "${git_commands[$cmd]}" ]]; then
    # Special handling for commands that need parameters
    if [[ "$cmd" == "c" || "$cmd" == "ac" ]]; then
      # For commit messages, pass all remaining args as a single string
      eval "git ${git_commands[$cmd]} \"$*\""
    elif [[ "$cmd" == "co" || "$cmd" == "nb" ]]; then
      # For checkout commands, pass the first parameter
      git ${git_commands[$cmd]} "$1"
    elif [[ "$cmd" == "nf" ]]; then
      # For feature branch creation
      git ${git_commands[$cmd]} "$1"
    else
      # For regular commands
      eval "git ${git_commands[$cmd]} $*"
    fi
  else
    # Check if it's a valid git command
    if git help "$cmd" &>/dev/null; then
      git "$cmd" "$@"
    else
      # Unknown command, show error
      echo "Error: Unknown git shortcut or command: '$cmd'"
      echo ""
      echo "Available shortcuts:"
      for cmd in "${!git_commands[@]}"; do
        echo "  $cmd => git ${git_commands[$cmd]}"
      done
      
      echo ""
      echo "Try 'git help' for a list of available git commands."
      return 1
    fi
  fi
}

# Tab completion for the 'g' function
_g_completion() {
  local cur=${COMP_WORDS[COMP_CWORD]}
  local prev=${COMP_WORDS[COMP_CWORD-1]}
  local git_shortcuts=()
  
  # First argument - command completion
  if [[ $COMP_CWORD -eq 1 ]]; then
    # Add all shorthand commands
    for cmd in "${!git_commands[@]}"; do
      git_shortcuts+=("$cmd")
    done
    
  elif [[ $COMP_CWORD -eq 2 ]]; then
    # Second argument - branch completion for relevant commands
    case "$prev" in
      co|checkout|nb|nf)
        # Get branch list from git
        local branches=$(git branch 2>/dev/null | grep -v '^*' | sed 's/^[[:space:]]*//')
        COMPREPLY=($(compgen -W "$branches" -- "$cur"))
        return 0
        ;;
    esac
  fi
  
  # Set completion reply
  COMPREPLY=($(compgen -W "${git_shortcuts[*]}" -- "$cur"))
  return 0
}

complete -F _g_completion g