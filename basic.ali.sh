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
