#!/usr/bin/env bash
# AWS helpers.

# ── Pick and activate an AWS profile ───────────────────────────────────────
# Usage: pickaws
# Requires: aws-cli; fzf is used when available, otherwise falls back to
# bash select.
pickaws() {
    local p profiles
    mapfile -t profiles < <(aws configure list-profiles)

    if [[ ${#profiles[@]} -eq 0 ]]; then
        echo "No AWS profiles found" >&2
        return 1
    fi

    if command -v fzf &>/dev/null; then
        p=$(printf '%s\n' "${profiles[@]}" | fzf)
    else
        echo "Select an AWS profile:"
        select p in "${profiles[@]}"; do
            [[ -n "$p" ]] && break
            echo "Invalid selection, try again"
        done
    fi

    [[ -z "$p" ]] && return 0

    export AWS_PROFILE="$p"
    echo "Profile: $p"

    if aws sts get-caller-identity --profile "$p" &>/dev/null; then
        echo -e "\033[32m[OK] Aktiv\033[0m"
    else
        echo -e "\033[33m[WARN] SSO Login erforderlich\033[0m"
        aws sso login --profile "$p"
    fi
}
