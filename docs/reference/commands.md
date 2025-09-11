# Command Reference

Complete reference for all aliases-cli commands and options.

## Global Options

```bash
aliases-cli [GLOBAL_OPTIONS] <command> [COMMAND_OPTIONS] [ARGUMENTS]
```

| Option | Description |
|--------|-------------|
| `--help`, `-h` | Show help message |
| `--version`, `-v` | Show version information |

## Commands Overview

| Command | Alias | Description |
|---------|-------|-------------|
| [`code`](#code-command) | `c` | Navigate to projects and components |
| [`update`](#update-command) | `uw` | Update git repositories |
| [`todo`](#todo-command) | - | Manage todos (TUI + CLI) |
| [`env`](#env-command) | `project_env` | Setup environment variables |

---

## `code` Command

Navigate to project directories and components.

### Syntax

```bash
aliases-cli code [OPTIONS] [PROJECT...] [COMPONENT]
# Alias: c [OPTIONS] [PROJECT...] [COMPONENT]
```

### Options

| Option | Description |
|--------|-------------|
| `--help`, `-h` | Show help for code command |
| `--list`, `-l` | List available projects |
| `--verbose`, `-v` | Verbose output |

### Arguments

| Argument | Description | Example |
|----------|-------------|---------|
| `PROJECT` | Project name or shortcut | `dispatch`, `dip` |
| `COMPONENT` | Component to open | `server`, `web`, `s`, `w` |

### Examples

#### Basic Navigation
```bash
# Navigate to project home directory
c dispatch                    # Go to ~/workspaces/dispatch/
c dip                        # Using shortcut (if configured)

# Navigate to specific component
c dispatch server            # Go to server component
c dispatch web              # Go to web component
c dip s                     # Shortcut + component abbreviation
c dip w                     # Web component
```

#### Multiple Components
```bash
# Bracket notation (opens multiple terminals/tabs)
c dispatch[sw]              # Open both server and web
c dip[sw]                   # Using shortcut

# Multiple projects
c project1 project2         # Open multiple projects
c dip urm                   # Open dispatch and urm projects
```

#### Listing and Discovery
```bash
c --list                    # List all available projects
c dispatch --list           # List components for project
c -l                        # Short form
```

### Component Resolution

The system searches for components in this order:

**Server component (`s`, `server`):**
1. Paths from `mappings.json` → `server_paths`
2. Default paths: `dispatch-server`, `server`, `java`, `serverJava`

**Web component (`w`, `web`):**
1. Paths from `mappings.json` → `web_paths` 
2. Default paths: `dispatch-web`, `web`, `webapp`, `webApp`, `frontend`

### Tab Completion

Smart tab completion supports:
- Project names and shortcuts
- Component abbreviations 
- Bracket notation
- Multiple project selection

```bash
c di<TAB>                   # Completes to available projects
c dispatch <TAB>            # Shows: server, web, s, w
c dispatch[<TAB>            # Shows bracket completions
```

---

## `update` Command  

Update git repositories in parallel.

### Syntax

```bash
aliases-cli update [OPTIONS] [PROJECT...]
# Alias: uw [OPTIONS] [PROJECT...]
```

### Options

| Option | Description | Default |
|--------|-------------|---------|
| `--help`, `-h` | Show help for update command | |
| `--verbose`, `-v` | Verbose output with git details | |
| `--jobs`, `-j N` | Number of parallel jobs | CPU cores |
| `--dry-run`, `-n` | Show what would be updated | |

### Arguments

| Argument | Description |
|----------|-------------|
| `PROJECT` | Specific projects to update (default: all) |

### Examples

#### Basic Updates
```bash
uw                          # Update all projects
uw dispatch                 # Update specific project
uw dispatch urm             # Update multiple projects
```

#### Advanced Options
```bash
uw --verbose                # Show git output
uw --jobs 4                 # Use 4 parallel jobs
uw --dry-run                # Preview changes
uw -v -j 8                  # Verbose with 8 jobs
```

#### Component-Specific Updates
```bash
uw dispatch server          # Update only server component
uw dispatch web             # Update only web component
uw dispatch[sw]             # Update both components
```

### Update Process

For each repository:
1. **Check git status** - Verify it's a clean working directory
2. **Fetch changes** - `git fetch origin`
3. **Merge updates** - `git pull` if fast-forward possible
4. **Report status** - Success, conflicts, or errors

### Output Formats

**Normal output:**
```
✓ dispatch: Updated successfully
✓ urm: Already up to date  
✗ project3: Merge conflicts detected
```

**Verbose output:**
```
[dispatch] Fetching from origin...
[dispatch] Fast-forward merge successful
[dispatch] 3 files changed, 15 insertions(+), 8 deletions(-)
✓ dispatch: Updated successfully
```

---

## `todo` Command

Manage todos with both TUI and CLI interfaces.

### Syntax

```bash
aliases-cli todo [SUBCOMMAND] [OPTIONS] [ARGUMENTS]
```

### Subcommands

| Subcommand | Description |
|------------|-------------|
| *(none)* | Launch interactive TUI mode |
| [`add`](#todo-add) | Add new todo |
| [`list`](#todo-list) | List todos |
| [`done`](#todo-done) | Mark todo as completed |
| [`remove`](#todo-remove) | Remove todo |
| [`priority`](#todo-priority) | Set todo priority |
| [`category`](#todo-category) | Set todo category |
| [`tui`](#todo-tui) | Launch TUI mode explicitly |

### Global Options

| Option | Description |
|--------|-------------|
| `--help`, `-h` | Show help for todo command |
| `--interactive`, `-i` | Launch TUI mode |

---

### `todo add`

Add a new todo item.

#### Syntax
```bash
aliases-cli todo add <DESCRIPTION>
```

#### Examples
```bash
aliases-cli todo add "Fix authentication bug"
aliases-cli todo add Implement user dashboard
aliases-cli todo add "Review PR #123"
```

---

### `todo list`

List todo items.

#### Syntax
```bash
aliases-cli todo list [OPTIONS]
# Alias: aliases-cli todo ls
```

#### Examples
```bash
aliases-cli todo list           # List active todos
aliases-cli todo ls             # Short form
```

#### Output Format
```
Active todos:

#3 🔴 [bug] Fix authentication system
#1 🟡 [feature] Implement user dashboard  
#2 🟢 Complete documentation
```

---

### `todo done`

Mark a todo as completed.

#### Syntax
```bash
aliases-cli todo done <ID>
# Alias: aliases-cli todo complete <ID>
```

#### Examples
```bash
aliases-cli todo done 1         # Complete todo #1
aliases-cli todo complete 3     # Alternative command
```

---

### `todo remove`

Remove a todo permanently.

#### Syntax
```bash
aliases-cli todo remove <ID>
# Aliases: rm, delete
```

#### Examples
```bash
aliases-cli todo remove 1       # Remove todo #1
aliases-cli todo rm 2           # Short form
aliases-cli todo delete 3       # Alternative
```

---

### `todo priority`

Set todo priority level.

#### Syntax
```bash
aliases-cli todo priority <ID> <PRIORITY>
# Alias: aliases-cli todo prio <ID> <PRIORITY>
```

#### Priority Levels
| Level | Indicator | Description |
|-------|-----------|-------------|
| `0` | *(none)* | No priority |
| `1` | 🟢 | Low priority |
| `2` | 🟡 | Medium priority |
| `3` | 🔴 | High priority |

#### Examples
```bash
aliases-cli todo priority 1 3   # Set high priority
aliases-cli todo prio 2 0       # Remove priority
```

---

### `todo category`

Set todo category.

#### Syntax
```bash
aliases-cli todo category <ID> <CATEGORY>
# Alias: aliases-cli todo cat <ID> <CATEGORY>
```

#### Examples
```bash
aliases-cli todo category 1 bug     # Set category
aliases-cli todo cat 2 feature      # Short form
```

---

### `todo tui`

Launch interactive TUI mode.

#### Syntax
```bash
aliases-cli todo tui
# Aliases: aliases-cli todo -i, aliases-cli todo --interactive
```

#### TUI Controls

| Key | Action |
|-----|--------|
| `↑↓` or `j k` | Navigate todos |
| `Space/Enter` | Toggle completion |
| `a` | Add new todo |
| `e` | Edit selected todo |
| `d` or `Del` | Delete todo |
| `c` | Toggle completed view |
| `r` | Refresh from disk |
| `q` | Quit |

---

## `env` Command

Setup project environment variables.

### Syntax

```bash
aliases-cli env [OPTIONS]
```

### Options

| Option | Description | Default |
|--------|-------------|---------|
| `--help`, `-h` | Show help for env command | |
| `--port`, `-p N` | Base port number | Auto-detect |
| `--environment`, `-e ENV` | Environment profile | `dev` |
| `--secure`, `-s BOOL` | Enable HTTPS | `false` |
| `--no-port`, `-n` | Disable port offset | |

### Examples

```bash
aliases-cli env                 # Auto-detect and setup
aliases-cli env -p 3000         # Use port 3000 as base
aliases-cli env -e prod         # Production environment
aliases-cli env -s true         # Enable HTTPS
project_env                     # Bash wrapper (recommended)
```

### Environment Variables Set

The command sets various project-specific environment variables:
- `PROJECT_NAME` - Current project name
- `PROJECT_ROOT` - Project root directory  
- `SERVER_PORT`, `WEB_PORT` - Component-specific ports
- `NODE_ENV` - Environment (dev, prod, test)
- Additional project-specific variables

### Integration

**Bash wrapper** (`project_env` function):
- Calls the C++ tool to generate export statements
- Evaluates them in the current shell
- Required because child processes can't modify parent environment

---

## Exit Codes

All commands return standard exit codes:

| Code | Meaning |
|------|---------|
| `0` | Success |
| `1` | General error |
| `2` | Invalid arguments |
| `127` | Command not found |

## Error Handling

Commands provide user-friendly error messages:

```bash
# Project not found
$ c nonexistent
✗ Project 'nonexistent' not found
Available projects: dispatch, urm, project3

# Invalid todo ID  
$ aliases-cli todo done 999
✗ Todo not found

# Git repository issues
$ uw project-with-conflicts
✗ project-with-conflicts: Merge conflicts detected
  Please resolve conflicts manually
```

## Configuration Files

Commands read configuration from:
- `mappings.json` - Project mappings and shortcuts
- `~/.config/aliases-cli/todos.json` - Todo data storage

See [Configuration Guide](../user-guide/configuration.md) for details.