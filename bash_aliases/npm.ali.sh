#!/bin/bash
##############################################################################
#                              NPM SHORTCUTS                                 #
##############################################################################

alias dev='npm run dev'
alias gen='npm run gen'
alias build='npm run build'
alias blocal='npm run build:local'
alias bstatic='npm run build:static'
alias vit='npm run vitest'
alias jes='npm run test:watch'
alias story='npm run start:storybook'
alias npmi='npm i'
alias lint='npm run lint'
alias pipeline='npm run pipeline:dry-run'

# Clean install - removes .next, node_modules, and package-lock.json before npm install
npm-clean() {
    [ -d ".next" ] && echo "  Removing .next..." && rm -rf .next
    [ -d "node_modules" ] && echo "  Removing node_modules..." && rm -rf node_modules
    [ -f "package-lock.json" ] && echo "  Removing package-lock.json..." && rm -rf package-lock.json
    npm i
}

# Package manager - alias npm to pnpm if pnpm is available
if command -v pnpm >/dev/null 2>&1; then
    alias npm='pnpm'
fi

# npm wrapper that automatically adds --ignore-scripts
npm() {
  # Commands where --ignore-scripts is valid
  declare -A IGNORE_CMDS=(
    [install]=1
    [i]=1
    [update]=1
    [up]=1
    [ci]=1
    [rebuild]=1
    [add]=1
    [remove]=1
    [rm]=1
    [link]=1
    [dedupe]=1
    [pack]=1
  )

  local cmd="$1"
  shift

  if [[ -n "${IGNORE_CMDS[$cmd]}" ]]; then
    # Command supports --ignore-scripts
    command npm "$cmd" --ignore-scripts "$@"
  else
    # Normal fallback
    command npm "$cmd" "$@"
  fi
}