#!/bin/bash
##############################################################################
#                              NPM SHORTCUTS                                 #
##############################################################################

alias gen='npm run gen'
alias build='npm run build:static'
alias dev='{ REPO_CMD=$(basename $(git rev-parse --show-toplevel 2>/dev/null)) && eval $REPO_CMD || echo ""; } && npm run dev'