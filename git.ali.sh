##############################################################################
#                               GIT SHORTCUTS                                #
##############################################################################

g() {
  # No parameters, show help
  if [[ $# -eq 0 ]]; then
    echo "Git shortcuts:"
    echo "  g s       - git status"
    echo "  g a       - git add ."
    echo "  g c \"msg\" - git commit -m \"msg\""
    echo "  g ac \"msg\"- git add . && git commit -m \"$*\""
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

  case "$1" in
    s)  git status ;;
    a)  git add . ;;
    c)  shift; git commit -m "$*" ;;
    ac) shift; git add . && git commit -m "$*" ;;
    p)  git pull ;;
    ps) git push ;;
    psf) git push -f ;;
    co) git checkout "$2" ;;
    nb) git checkout -b "$2" ;;
    nf) git checkout -b "features/$2" ;;
    l)  git log --oneline ;;
    b)  git branch ;;
    *)  git "$@" ;;
  esac
}