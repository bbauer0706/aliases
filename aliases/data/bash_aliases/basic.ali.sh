#!/usr/bin/env bash
# Basic shell aliases and utilities.

# ── Directory navigation ────────────────────────────────────────────────────
alias la='ls -alh'
alias ..='cd ..'
alias ...='cd ../..'
# shellcheck disable=SC2139
alias ~='cd ~'

# ── Kill process on port ────────────────────────────────────────────────────
# Usage: kp 3000    or    kp 3000/tcp
kp() {
    fuser -k "${1}/tcp"
}

# ── TypeScript check (no emit) ──────────────────────────────────────────────
alias tsc='npx tsc --noEmit'

# ── Show current project environment variables ──────────────────────────────
# (delegates to the CLI; defined in shell/project-env.sh as well)
show_env() {
    aliases env --show
}
