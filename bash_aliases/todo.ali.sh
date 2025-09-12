##############################################################################
#                            TODO ALIASES                                   #
##############################################################################

# Main todo alias with smart parameter handling
# Usage:
#   td                    - Launch interactive TUI
#   td "task"             - Add todo with description
#   td category "task"    - Add todo with category and description  
#   td command args...    - Pass through to main command
td() {
    case $# in
        0)
            # No arguments - launch interactive TUI
            aliases-cli todo
            ;;
        1)
            # One argument - add todo with description
            aliases-cli todo add "$1"
            ;;
        2)
            # Two arguments - add todo with category and description
            aliases-cli todo add "$2" -c "$1"
            ;;
        *)
            # More arguments - pass through to main command
            aliases-cli todo "$@"
            ;;
    esac
}

# Legacy alias for backward compatibility
alias todo='td'

# Quick status check
alias tdl='aliases-cli todo list'    # List todos
alias tds='aliases-cli todo search'  # Search todos

# Priority-based todo shortcuts
td-high() {
    aliases-cli todo add "$*" -p 3
}

td-med() {
    aliases-cli todo add "$*" -p 2
}

td-low() {
    aliases-cli todo add "$*" -p 1
}

# Category-based shortcuts
td-bug() {
    aliases-cli todo add "$*" -p 3 -c bug
}

td-feature() {
    aliases-cli todo add "$*" -c feature
}

td-docs() {
    aliases-cli todo add "$*" -c docs
}

# Additional category shortcuts for common use cases
td-review() {
    aliases-cli todo add "$*" -p 2 -c review
}

td-deploy() {
    aliases-cli todo add "$*" -p 3 -c deployment
}

td-test() {
    aliases-cli todo add "$*" -c testing
}

# Legacy aliases for backward compatibility
alias todo-high='td-high'
alias todo-med='td-med'
alias todo-low='td-low'
alias todo-bug='td-bug'
alias todo-feature='td-feature'
alias todo-docs='td-docs'
alias todo-review='td-review'
alias todo-deploy='td-deploy'
alias todo-test='td-test'
alias todos='tdl'

# Search-based todo completion functions
td-done() {
    if [ $# -eq 0 ]; then
        echo "Usage: td-done <search_term>"
        echo "Example: td-done \"auth bug\""
        return 1
    fi
    
    local search_query="$*"
    local todo_id=$(aliases-cli todo search "$search_query" --id-only)
    
    if [ $? -eq 0 ] && [ -n "$todo_id" ]; then
        aliases-cli todo done "$todo_id"
    else
        echo "No todo found matching: $search_query"
        return 1
    fi
}

# Search-based todo removal
td-rm() {
    if [ $# -eq 0 ]; then
        echo "Usage: td-rm <search_term>"
        echo "Example: td-rm \"old feature\""
        return 1
    fi
    
    local search_query="$*"
    local todo_id=$(aliases-cli todo search "$search_query" --id-only)
    
    if [ $? -eq 0 ] && [ -n "$todo_id" ]; then
        aliases-cli todo remove "$todo_id"
    else
        echo "No todo found matching: $search_query"
        return 1
    fi
}

# Search-based priority setting
td-urgent() {
    if [ $# -eq 0 ]; then
        echo "Usage: td-urgent <search_term>"
        echo "Example: td-urgent \"production bug\""
        return 1
    fi
    
    local search_query="$*"
    local todo_id=$(aliases-cli todo search "$search_query" --id-only)
    
    if [ $? -eq 0 ] && [ -n "$todo_id" ]; then
        aliases-cli todo priority "$todo_id" 3
    else
        echo "No todo found matching: $search_query"
        return 1
    fi
}

# Category-based search shortcuts
td-find() {
    if [ $# -eq 0 ]; then
        echo "Usage: td-find <search_term> [category]"
        echo "Example: td-find \"auth\" bug"
        return 1
    fi
    
    local search_query="$1"
    shift
    
    if [ $# -gt 0 ]; then
        local category="$1"
        aliases-cli todo search "$search_query" -c "$category"
    else
        aliases-cli todo search "$search_query"
    fi
}

# Find todos by category only
td-bugs() {
    aliases-cli todo search "" -c bug 2>/dev/null || aliases-cli todo list | grep "\\[bug\\]"
}

td-features() {
    aliases-cli todo search "" -c feature 2>/dev/null || aliases-cli todo list | grep "\\[feature\\]"
}

td-reviews() {
    aliases-cli todo search "" -c review 2>/dev/null || aliases-cli todo list | grep "\\[review\\]"
}

# Smart completion - finds and completes first high-priority match
td-next() {
    local todo_id=$(aliases-cli todo list | grep "🔴" | head -1 | sed 's/.*#\([0-9]*\).*/\1/')
    
    if [ -n "$todo_id" ]; then
        echo "Completing high-priority todo #$todo_id"
        aliases-cli todo done "$todo_id"
    else
        # No high priority, try medium
        todo_id=$(aliases-cli todo list | grep "🟡" | head -1 | sed 's/.*#\([0-9]*\).*/\1/')
        if [ -n "$todo_id" ]; then
            echo "Completing medium-priority todo #$todo_id"
            aliases-cli todo done "$todo_id"
        else
            echo "No prioritized todos found"
        fi
    fi
}

# Git branch-based todo creation
td-branch() {
    if [ $# -eq 0 ]; then
        echo "Usage: td-branch <description>"
        echo "Creates a todo with git project name as category and branch name in description"
        echo "Example: td-branch \"Fix login validation\""
        return 1
    fi
    
    # Check if we're in a git repository
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        echo "Error: Not in a git repository"
        return 1
    fi
    
    # Get current branch name
    local branch_name=$(git branch --show-current 2>/dev/null)
    
    if [ -z "$branch_name" ]; then
        echo "Error: Could not determine current branch"
        return 1
    fi
    
    # Get git project name from remote URL or directory name
    local project_name=$(git remote get-url origin 2>/dev/null | sed 's/.*\/\([^/]*\)\.git$/\1/' | sed 's/.*\/\([^/]*\)$/\1/')
    
    # If no remote origin, use the directory name of the git root
    if [ -z "$project_name" ]; then
        project_name=$(basename "$(git rev-parse --show-toplevel)" 2>/dev/null)
    fi
    
    # If still no project name, fallback to "git-project"
    if [ -z "$project_name" ]; then
        project_name="git-project"
    fi
    
    # Create todo with project name as category and branch name prepended to description
    local description="$*"
    local full_description="$branch_name: $description"
    
    echo "Creating todo for project: $project_name (branch: $branch_name)"
    aliases-cli todo add "$full_description" -c "$project_name" -p 2
}

# Alias for convenience
alias tdb='td-branch'

# Show todos for current git project and branch
td-branch-list() {
    # Check if we're in a git repository
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        echo "Error: Not in a git repository"
        return 1
    fi
    
    # Get current branch name
    local branch_name=$(git branch --show-current 2>/dev/null)
    
    if [ -z "$branch_name" ]; then
        echo "Error: Could not determine current branch"
        return 1
    fi
    
    # Get git project name (same logic as td-branch)
    local project_name=$(git remote get-url origin 2>/dev/null | sed 's/.*\/\([^/]*\)\.git$/\1/' | sed 's/.*\/\([^/]*\)$/\1/')
    
    if [ -z "$project_name" ]; then
        project_name=$(basename "$(git rev-parse --show-toplevel)" 2>/dev/null)
    fi
    
    if [ -z "$project_name" ]; then
        project_name="git-project"
    fi
    
    echo "Todos for project: $project_name, branch: $branch_name"
    
    # Search for todos in the project category that contain the branch name
    aliases-cli todo search "$branch_name:" -c "$project_name" 2>/dev/null || {
        echo "No todos found for project '$project_name' and branch '$branch_name'"
        return 0
    }
}

# Alias for convenience
alias tdbl='td-branch-list'

# Show all todos for current git project
td-project() {
    # Check if we're in a git repository
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        echo "Error: Not in a git repository"
        return 1
    fi
    
    # Get git project name (same logic as td-branch)
    local project_name=$(git remote get-url origin 2>/dev/null | sed 's/.*\/\([^/]*\)\.git$/\1/' | sed 's/.*\/\([^/]*\)$/\1/')
    
    if [ -z "$project_name" ]; then
        project_name=$(basename "$(git rev-parse --show-toplevel)" 2>/dev/null)
    fi
    
    if [ -z "$project_name" ]; then
        project_name="git-project"
    fi
    
    echo "All todos for project: $project_name"
    aliases-cli todo search "" -c "$project_name" 2>/dev/null || {
        echo "No todos found for project: $project_name"
        return 0
    }
}

# Alias for convenience
alias tdp='td-project'

# Legacy aliases for backward compatibility
alias todo-done='td-done'
alias todo-rm='td-rm'
alias todo-urgent='td-urgent'
alias todo-find='td-find'
alias todo-bugs='td-bugs'
alias todo-features='td-features'
alias todo-reviews='td-reviews'
alias todo-next='td-next'
alias todo-branch='td-branch'
alias todo-branch-list='td-branch-list'
alias todo-project='td-project'
