#!/bin/bash
##############################################################################
#                                                                            #
#                          BASIC BASH ALIASES                                #
#                                                                            #
##############################################################################

# Directory listing
alias la='ls -A'
alias ll='ls -l'

# Kill process on port
kp() {
  fuser -k $1/tcp
}

# Edit configuration files
alias ali='code ~/.bash_aliases'
