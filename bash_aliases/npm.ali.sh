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