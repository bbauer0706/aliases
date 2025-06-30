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

# Kill process on port
kp() {
  fuser -k $1/tcp
}

# Edit configuration files
alias ali='code ~/.bash_aliases'
