##############################################################################
#                            TODO ALIASES                                   #
##############################################################################

# Quick todo aliases for efficient task management
alias t='aliases-cli todo'              # Main todo command
alias tl='aliases-cli todo list'         # List todos
alias td='aliases-cli todo done'         # Mark todo done
alias tr='aliases-cli todo remove'       # Remove todo
alias tp='aliases-cli todo priority'     # Set priority
alias tc='aliases-cli todo category'     # Set category

# Quick todo add function - allows natural language
todo() {
    if [ $# -eq 0 ]; then
        # No arguments - launch TUI
        aliases-cli todo
    else
        # Arguments provided - add new todo
        aliases-cli todo add "$*"
    fi
}

# Priority-based todo shortcuts
todo-high() {
    local id=$(aliases-cli todo add "$*" | grep -o '#[0-9]*' | tr -d '#')
    if [ -n "$id" ]; then
        aliases-cli todo priority "$id" 3
        echo "Added high-priority todo #$id: $*"
    fi
}

todo-med() {
    local id=$(aliases-cli todo add "$*" | grep -o '#[0-9]*' | tr -d '#')
    if [ -n "$id" ]; then
        aliases-cli todo priority "$id" 2
        echo "Added medium-priority todo #$id: $*"
    fi
}

todo-low() {
    local id=$(aliases-cli todo add "$*" | grep -o '#[0-9]*' | tr -d '#')
    if [ -n "$id" ]; then
        aliases-cli todo priority "$id" 1
        echo "Added low-priority todo #$id: $*"
    fi
}

# Category-based shortcuts
todo-bug() {
    local id=$(aliases-cli todo add "$*" | grep -o '#[0-9]*' | tr -d '#')
    if [ -n "$id" ]; then
        aliases-cli todo category "$id" "bug"
        aliases-cli todo priority "$id" 3  # High priority for bugs
        echo "Added bug todo #$id: $*"
    fi
}

todo-feature() {
    local id=$(aliases-cli todo add "$*" | grep -o '#[0-9]*' | tr -d '#')
    if [ -n "$id" ]; then
        aliases-cli todo category "$id" "feature"
        echo "Added feature todo #$id: $*"
    fi
}

todo-docs() {
    local id=$(aliases-cli todo add "$*" | grep -o '#[0-9]*' | tr -d '#')
    if [ -n "$id" ]; then
        aliases-cli todo category "$id" "docs"
        echo "Added documentation todo #$id: $*"
    fi
}

# Quick status check
todos() {
    aliases-cli todo list
}
