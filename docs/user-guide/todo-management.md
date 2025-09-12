# Todo Management Guide

aliases-cli includes a powerful todo management system with both CLI and interactive TUI modes.

## Overview

The todo system provides:
- **Interactive TUI** with ncurses for visual management
- **CLI commands** for scripting and quick operations
- **JSON persistence** with automatic saving
- **Priority system** (0-3, with visual indicators)
- **Categories** for organization
- **Due dates** and completion tracking

## Quick Start

### Launch Interactive TUI
```bash
aliases-cli todo           # Launch TUI mode
# or
aliases-cli todo -i
```

### Basic CLI Operations
```bash
aliases-cli todo add "Complete project documentation"
aliases-cli todo list
aliases-cli todo done 1
```

## Interactive TUI Mode

The TUI provides a full-screen interface for managing todos visually.

### Launching TUI
```bash
aliases-cli todo           # Default: launch TUI if no arguments
aliases-cli todo tui       # Explicit TUI mode
aliases-cli todo -i        # Interactive flag
```

### TUI Controls

| Key | Action | Description |
|-----|--------|-------------|
| `Up/Down`, `j k`, or `w s` | Navigate | Move selection up/down |
| `Space` or `Enter` | Toggle | Mark todo as complete/incomplete |
| `n` | Add | Create new todo |
| `e` | Edit | Edit selected todo description |
| `x` or `Del` | Delete | Remove selected todo |
| `d` or `Right` | Priority Up | Increase selected todo's priority |
| `a` or `Left` | Priority Down | Decrease selected todo's priority |
| `+` or `=` | Priority Up | Increase selected todo's priority (alternative) |
| `-` or `_` | Priority Down | Decrease selected todo's priority (alternative) |
| `c` | Toggle View | Show/hide completed todos |
| `f` or `l` | Filter | Open category filter selection |
| `o` | Sort | Toggle between Priority and Index (creation order) sorting |
| `r` | Refresh | Reload todos from disk |
| `q` or `Q` | Quit | Exit TUI mode |

### TUI Features

**Visual Indicators:**
- `[ ]` - Incomplete todo
- `[x]` - Completed todo
- `!!!` - High priority (3) - **Red color**
- `!!` - Medium priority (2) - **Yellow color**
- `!` - Low priority (1) - **Blue color**
- `[category]` - Category labels
- **Note**: Priority colors are shown when items are not selected or completed

**Status Bar:**
Shows active vs completed counts and current view mode.

**Edit Mode:**
- `Enter` - Save changes
- `Esc` - Cancel editing
- `Backspace` - Delete characters
- `Tab` - Switch between editing description and category (existing todos only)
- Type normally to add text

**Edit Mode Features:**
- When editing existing todos, use `Tab` to switch between description and category editing
- New todos only support description editing initially

**Category Filtering:**
- Press `f` or `l` to open category filter
- `Up/Down` or `j k` - Navigate filter options
- `Space` - Toggle category selection (multi-select supported)
- `Enter` - Apply selected filters
- `Esc` - Cancel filter changes
- Select "All" to clear all category filters

**Priority Controls:**
- `d` or `Right` - Increase priority of selected todo
- `a` or `Left` - Decrease priority of selected todo  
- `+` or `=` - Increase priority of selected todo (alternative)
- `-` or `_` - Decrease priority of selected todo (alternative)
- Priorities: 0 (none), 1 (! low - blue), 2 (!! medium - yellow), 3 (!!! high - red)
- **Tip**: Items stay in place when adjusting priority to avoid jumping. Press `o` twice to re-sort if desired.

**Sort Modes:**
- Press `o` to toggle between sorting modes
- **Priority Sort (default)**: Sorts by priority (high to low), then by creation time
- **Index Sort**: Sorts by creation order (ID order), showing todos in the order they were created
- **Note**: Priority changes don't immediately re-sort the list to prevent items jumping around. Press `o` twice (or use other operations like toggle completion) to trigger re-sorting.

## CLI Commands

### Adding Todos

```bash
# Basic todo
aliases-cli todo add "Fix authentication bug"

# Todo with priority and category
aliases-cli todo add "Fix authentication bug" -p 3 -c bug
aliases-cli todo add "Review PR" --priority 2 --category code-review
aliases-cli todo add "Deploy to staging" -p 3 -c deployment

# Todo with all arguments as description (no quotes needed)
aliases-cli todo add Fix the login system error handling
```

**Add Command Options:**
- `-p, --priority <0-3>` - Set priority (0=none, 1=low, 2=med, 3=high)
- `-c, --category <category>` - Set category

**Examples:**
```bash
aliases-cli todo add "Complete project documentation"                    # Basic todo
aliases-cli todo add "Fix user registration" -p 3                       # High priority
aliases-cli todo add "Update API docs" -c documentation                 # With category  
aliases-cli todo add "Security audit" -p 3 -c security                  # Priority + category
aliases-cli todo add "Code review for PR #123" --priority 2 --category review  # Long form
```

### Listing Todos

```bash
# List active todos (sorted by priority, then creation time)
aliases-cli todo list

# Alternative command
aliases-cli todo ls
```

**Example Output:**
```
Active todos:

#3 !!! [bug] Fix authentication system     (high priority - red)
#1 !! [feature] Implement user dashboard   (medium priority - yellow)
#2 ! Complete documentation               (low priority - blue)
```

### Completing Todos

```bash
# Mark todo as complete
aliases-cli todo done 1

# Alternative command
aliases-cli todo complete 1
```

### Removing Todos

```bash
# Remove todo permanently
aliases-cli todo remove 1

# Alternative commands
aliases-cli todo rm 1
aliases-cli todo delete 1
```

### Priority Management

```bash
# Set priority (0-3)
aliases-cli todo priority 1 3    # Set todo #1 to high priority (!!! - red)
aliases-cli todo priority 1 2    # Medium priority (!! - yellow)
aliases-cli todo priority 1 1    # Low priority (! - blue)  
aliases-cli todo priority 1 0    # No priority

# Alternative command
aliases-cli todo prio 1 2
```

**Priority Levels:**
- `0` - No priority (no indicator)
- `1` - Low priority (! - blue)
- `2` - Medium priority (!! - yellow)
- `3` - High priority (!!! - red)

### Category Management

```bash
# Set category
aliases-cli todo category 1 bug
aliases-cli todo category 2 feature

# Alternative command
aliases-cli todo cat 1 urgent
```

Categories appear as `[category]` labels in listings.

### Searching Todos

The search command allows you to find todos by description text and optionally filter by category:

```bash
# Basic search (case-insensitive)
aliases-cli todo search "authentication"
aliases-cli todo search "bug fix"

# Search with category filter
aliases-cli todo search "review" -c code-review
aliases-cli todo search "deploy" --category production

# Get only the ID for piping (returns highest priority match)
aliases-cli todo search "auth" --id-only

# Alternative command
aliases-cli todo find "authentication"
```

**Search Command Options:**
- `-c, --category <category>` - Filter search results by category
- `--id-only` - Output only the ID of the first (highest priority) match for piping

**Search Features:**
- **Case-insensitive**: Search terms match regardless of case
- **Priority sorting**: Results are sorted by priority (high to low), then by creation time
- **Active todos only**: Only searches incomplete todos
- **Partial matching**: Finds todos containing your search terms anywhere in the description

**Examples:**
```bash
# Find todos containing "authentication"
aliases-cli todo search "authentication"
# Output: Found 2 todo(s) matching 'authentication':
#         #5 🔴 [bug] Fix authentication system bug
#         #8 🟡 Update authentication documentation

# Search within a specific category
aliases-cli todo search "review" -c code-review
# Output: Found 1 todo(s) matching 'review' in category 'code-review':
#         #12 🟡 [code-review] Review user management PR

# Get ID for command chaining (pipe-friendly)
aliases-cli todo search "auth bug" --id-only
# Output: 5

# Use with other commands
aliases-cli todo done $(aliases-cli todo search "auth" --id-only)
```

### Help

```bash
aliases-cli todo --help
aliases-cli todo -h
aliases-cli todo help
```

## Data Storage

Todos are stored in JSON format at:
```
~/.config/aliases-cli/todos.json
```

### Data Structure

```json
{
  "todos": [
    {
      "id": 1,
      "description": "Fix authentication bug",
      "completed": false,
      "priority": 3,
      "category": "bug",
      "created_at": 1694428800,
      "completed_at": null,
      "due_date": null
    }
  ],
  "next_id": 2
}
```

### Backup and Migration

```bash
# Backup todos
cp ~/.config/aliases-cli/todos.json ~/todos-backup.json

# Restore todos
cp ~/todos-backup.json ~/.config/aliases-cli/todos.json
```

## Integration with Other Tools

### Shell Aliases

Add convenient aliases to your shell:

```bash
# Quick shortcuts
alias t='aliases-cli todo'
alias ta='aliases-cli todo add'
alias tl='aliases-cli todo list' 
alias td='aliases-cli todo done'
alias ts='aliases-cli todo search'
```

### Advanced Aliases with Search

The aliases-cli includes powerful search-based aliases for efficient todo management:

**Search-Based Completion:**
```bash
# Complete todo by search term (finds highest priority match)
todo-done "authentication bug"
todo-done "deploy feature"

# Remove todo by search term
todo-rm "old feature request" 

# Mark todo as urgent by search term
todo-urgent "production issue"
```

**Category-Specific Search:**
```bash
# Find todos in specific categories
todo-bugs          # Show all bug-category todos
todo-features      # Show all feature-category todos  
todo-reviews       # Show all review-category todos

# Advanced search with category filter
todo-find "auth" bug           # Search "auth" in bug category
todo-find "deploy" production  # Search "deploy" in production category
```

**Smart Workflows:**
```bash
# Complete next highest priority todo automatically
todo-next          # Completes first high-priority, then medium-priority todo

# Chain operations with search
aliases-cli todo done $(aliases-cli todo search "auth" --id-only)
aliases-cli todo priority $(aliases-cli todo search "urgent" --id-only) 3
```

**Example Usage:**
```bash
# Instead of: aliases-cli todo list, find ID, then aliases-cli todo done 5
# Just use:
todo-done "authentication"     # Finds and completes first match

# Instead of: aliases-cli todo search "bug", remember ID, set priority
# Just use:  
todo-urgent "critical bug"     # Finds and marks as high priority

# Quickly complete high-priority work
todo-next                      # Completes next prioritized todo automatically
```

### Git Hooks

Integrate with git workflows:

```bash
# .git/hooks/post-commit
#!/bin/bash
aliases-cli todo add "Review commit $(git rev-parse --short HEAD)"
```

### Scripts and Automation

The CLI interface makes it easy to integrate with scripts:

```bash
#!/bin/bash
# Add todos from a project plan
while IFS= read -r task; do
    aliases-cli todo add "$task"
done < project-tasks.txt
```

## Troubleshooting

### TUI Not Available

If you see "Interactive TUI mode is not available":

1. **Check ncurses**: The system should include ncurses, but verify with:
   ```bash
   ./build.sh  # Should show "Local ncurses found"
   ```

2. **Terminal compatibility**: Ensure your terminal supports ncurses:
   ```bash
   echo $TERM  # Should show something like 'xterm-256color'
   ```

3. **Fall back to CLI**: Use CLI commands if TUI is unavailable

### Data Corruption

If todos don't load properly:

1. **Check JSON syntax**:
   ```bash
   cat ~/.config/aliases-cli/todos.json | python -m json.tool
   ```

2. **Reset if needed**:
   ```bash
   rm ~/.config/aliases-cli/todos.json  # Will recreate empty
   ```

### Performance Issues

For large numbers of todos:
- The TUI efficiently handles hundreds of todos
- CLI operations are very fast (< 1ms)
- Consider archiving completed todos if needed

## Advanced Features

### Search and Filtering

**Current Features:**
- ✅ Search by description: `aliases-cli todo search "authentication"`
- ✅ Filter search by category: `aliases-cli todo search "bug" -c production`
- ✅ ID-only output for piping: `aliases-cli todo search "auth" --id-only`
- ✅ Priority-based result sorting (highest priority first)

**Planned Features:**
- Filter by category: `aliases-cli todo list --category bug`
- Filter by priority: `aliases-cli todo list --priority 3`
- Advanced search operators: `aliases-cli todo search "bug AND auth"`

### Due Dates (Future)

Planned support for:
- Setting due dates: `aliases-cli todo due 1 "2023-12-31"`
- Overdue highlighting in TUI
- Due date sorting

### Export/Import (Future)

Planned formats:
- Markdown export for documentation
- CSV export for spreadsheet integration
- Import from other todo systems