#!/usr/bin/env bash
# Shell integration for aliases-cli – secrets export helper.
# Source this file from ~/.bash_aliases (done automatically by aliases-cli setup).
#
# Usage:
#   secrets_load               # export all stored secrets
#   secrets_load MY_TOKEN      # export only MY_TOKEN
#   secrets_load A B C         # export A, B, and C
#
# Secrets are stored in the OS keychain (GNOME Keyring, macOS Keychain, etc.).
# No master password is required at load time; the keychain handles auth.

secrets_load() {
    local output
    output=$(aliases-cli secrets load "$@") || return $?
    eval "$output"
}

alias sload='secrets_load'
