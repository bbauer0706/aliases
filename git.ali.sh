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
  # No parameters, show help
  if [[ $# -eq 0 ]]; then
    echo "Git shortcuts:"
    echo "  g s       - git status"
    echo "  g a       - git add ."
    echo "  g c \"msg\" - git commit -m \"msg\""
    echo "  g ac \"msg\"- git add . && git commit -m \"msg\""
    echo "  g p       - git pull"
    echo "  g ps      - git push"
    echo "  g psf     - git push -f"
    echo "  g co br   - git checkout branch"
    echo "  g nb br   - git checkout -b branch"
    echo "  g nf br   - git checkout -b features/branch"
    echo "  g l       - git log --oneline"
    echo "  g b       - git branch"
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
    *)  git "$cmd" "$@" ;;
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