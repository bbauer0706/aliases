#!/usr/bin/env bash
# bash_integration/secrets.sh
#
# Bash integration for `aliases secrets`.
#
# Source this file (e.g. from ~/.bashrc) to get the `secrets_load` helper,
# which decrypts the secrets store and exports all (or named) variables into
# the current shell session.
#
# Usage:
#   secrets_load               # export all stored secrets into the current shell
#   secrets_load MY_TOKEN      # export only MY_TOKEN
#   secrets_load A B C         # export A, B and C
#
# The master password is read from $ALIASES_MASTER_PASSWORD if set;
# otherwise you will be prompted interactively.

secrets_load() {
    local output
    output=$(aliases-cli secrets load "$@") || return $?
    eval "$output"
}

# Alias for convenience
alias sload='secrets_load'
