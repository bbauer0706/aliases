#!/usr/bin/env bash
# Git helpers and safety wrappers.

alias reset-soft='git reset --soft HEAD~1'
alias git-rm='git rm --cached'
alias trigger-ci='git commit --allow-empty -m "ci: trigger ci pipeline" && git push'

# Safe rebase onto origin/main with pre-flight checks.
rebase() {
    if ! git rev-parse --is-inside-work-tree &>/dev/null; then
        echo "Error: not in a git repository" >&2
        return 1
    fi

    local branch
    branch=$(git rev-parse --abbrev-ref HEAD)

    if [[ "$branch" == "main" || "$branch" == "master" ]]; then
        echo "Error: cannot rebase the main/master branch" >&2
        return 1
    fi

    if ! git diff-index --quiet HEAD --; then
        echo "Error: uncommitted changes – commit or stash first" >&2
        return 1
    fi

    if ! git remote get-url origin &>/dev/null; then
        echo "Error: 'origin' remote not found" >&2
        return 1
    fi

    echo "Fetching origin/main…"
    git fetch origin main || { echo "Error: fetch failed" >&2; return 1; }

    echo "Rebasing '$branch' onto origin/main…"
    git rebase origin/main || {
        echo "Rebase failed – resolve conflicts then: git rebase --continue" >&2
        return 1
    }

    echo "Rebase successful."
    read -r -p "Force-push to origin/$branch? [y/N]: " -n 1
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        git push --force-with-lease origin "$branch" \
            && echo "Force-pushed to origin/$branch." \
            || { echo "Error: force-push failed" >&2; return 1; }
    else
        echo "Not pushed. Run: git push --force-with-lease origin $branch"
    fi
}
