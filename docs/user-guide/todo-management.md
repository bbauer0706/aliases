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
| `↑↓` or `j k` | Navigate | Move selection up/down |
| `Space` or `Enter` | Toggle | Mark todo as complete/incomplete |
| `a` | Add | Create new todo |
| `e` | Edit | Edit selected todo description |
| `d` or `Del` | Delete | Remove selected todo |
| `c` | Toggle View | Show/hide completed todos |
| `r` | Refresh | Reload todos from disk |
| `q` or `Q` | Quit | Exit TUI mode |

### TUI Features

**Visual Indicators:**
- `[ ]` - Incomplete todo
- `[✓]` - Completed todo
- `🔴` - High priority (3)
- `🟡` - Medium priority (2)
- `🟢` - Low priority (1)
- `[category]` - Category labels

**Status Bar:**
Shows active vs completed counts and current view mode.

**Edit Mode:**
- `Enter` - Save changes
- `Esc` - Cancel editing
- `Backspace` - Delete characters
- Type normally to add text

## CLI Commands

### Adding Todos

```bash
# Basic todo
aliases-cli todo add "Fix authentication bug"

# Todo with all arguments as description
aliases-cli todo add Fix the login system error handling
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

#3 🔴 [bug] Fix authentication system
#1 🟡 [feature] Implement user dashboard  
#2 🟢 Complete documentation
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
aliases-cli todo priority 1 3    # Set todo #1 to high priority (🔴)
aliases-cli todo priority 1 2    # Medium priority (🟡)
aliases-cli todo priority 1 1    # Low priority (🟢)  
aliases-cli todo priority 1 0    # No priority

# Alternative command
aliases-cli todo prio 1 2
```

**Priority Levels:**
- `0` - No priority (no indicator)
- `1` - Low priority (🟢)
- `2` - Medium priority (🟡)
- `3` - High priority (🔴)

### Category Management

```bash
# Set category
aliases-cli todo category 1 bug
aliases-cli todo category 2 feature

# Alternative command
aliases-cli todo cat 1 urgent
```

Categories appear as `[category]` labels in listings.

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

### Filtering (Future)

Planned features:
- Filter by category: `aliases-cli todo list --category bug`
- Filter by priority: `aliases-cli todo list --priority 3`
- Search: `aliases-cli todo search "authentication"`

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