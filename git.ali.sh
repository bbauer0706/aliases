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

  case "$cmd" in
    s)  git status ;;
    a)  git add . ;;
    c)  git commit -m "$*" ;;
    ac) git add . && git commit -m "$*" ;;
    p)  git pull ;;
    ps) git push ;;
    psf) git push -f ;;
    co) git checkout "$1" ;;
    nb) git checkout -b "$1" ;;
    nf) git checkout -b "features/$1" ;;
    l)  git log --oneline ;;
    b)  git branch ;;
    *)  
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
        ;;
  esac
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