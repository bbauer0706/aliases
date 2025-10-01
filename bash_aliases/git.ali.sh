#!/bin/bash
##############################################################################
#                                                                            #
#                            GIT BASH ALIASES                                #
#                                                                            #
##############################################################################


alias reset-soft='git reset --soft HEAD~1'

# Git rebase with safety checks
rebase() {
    # Check if we're in a git repository
    if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        echo "Error: Not in a git repository" >&2
        return 1
    fi
    
    # Get current branch name
    local current_branch=$(git rev-parse --abbrev-ref HEAD)
    
    # Don't allow rebasing main branch
    if [ "$current_branch" = "main" ] || [ "$current_branch" = "master" ]; then
        echo "Error: Cannot rebase the main/master branch" >&2
        return 1
    fi
    
    # Check if there are uncommitted changes
    if ! git diff-index --quiet HEAD --; then
        echo "Error: You have uncommitted changes. Please commit or stash them first." >&2
        return 1
    fi
    
    # Check if origin remote exists
    if ! git remote get-url origin >/dev/null 2>&1; then
        echo "Error: 'origin' remote not found" >&2
        return 1
    fi
    
    echo "Rebasing branch '$current_branch' onto origin/main..."
    
    # Fetch latest changes from origin
    echo "Fetching latest changes..."
    if ! git fetch origin main; then
        echo "Error: Failed to fetch from origin" >&2
        return 1
    fi
    
    # Perform the rebase
    echo "Rebasing onto origin/main..."
    if ! git rebase origin/main; then
        echo "Rebase failed. Please resolve conflicts and run 'git rebase --continue'" >&2
        return 1
    fi
    
    # Confirm force push
    echo "Rebase successful. Ready to force push to origin/$current_branch"
    read -p "Force push to origin? [y/N]: " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Force pushing to origin/$current_branch..."
        if git push -f origin "$current_branch"; then
            echo "Successfully rebased and force pushed!"
        else
            echo "Error: Force push failed" >&2
            return 1
        fi
    else
        echo "Rebase completed but not pushed. Run 'git push -f origin $current_branch' when ready."
    fi
}

alias git-rm='git rm --cached'