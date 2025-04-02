#!/bin/bash
# Git Remote Management Commands Reference

# Remove a remote
# Usage: git remote remove <remote-name>
# Example: git remote remove origin
git_remove_remote() {
  if [ -z "$1" ]; then
    echo "Usage: git_remove_remote <remote-name>"
    return 1
  fi
  git remote remove "$1"
}

# List all remotes
git_list_remotes() {
  git remote -v
}

# Add a new remote
# Usage: git_add_remote <name> <url>
git_add_remote() {
  if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Usage: git_add_remote <remote-name> <remote-url>"
    return 1
  fi
  git remote add "$1" "$2"
}

# You can source this file in your .bashrc or directly run the commands
# Example: git remote remove origin
