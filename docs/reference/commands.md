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
| [`todo`](#todo-command) | - | Manage todos (TUI + CLI) |
| [`config`](#config-command) | - | Manage aliases-cli configuration |
| [`env`](#env-command) | `project_env` | Setup environment variables |

---

## `code` Command

Navigate to project directories and components with intelligent fallback to VS Code.

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
| `PATH` | Any valid path or VS Code argument | `..`, `.`, `/path/to/dir` |

### Behavior

The `c` command intelligently determines how to handle arguments:

1. **Project shortcuts first**: Checks if the argument matches a configured project
2. **Fallback to VS Code**: If no project matches, passes arguments directly to `code` command

This allows seamless usage for both project shortcuts and regular VS Code operations.

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

#### Fallback to Regular VS Code
```bash
# When argument doesn't match a project, acts like 'code'
c ..                        # Open parent directory in VS Code
c .                         # Open current directory in VS Code
c /path/to/folder           # Open any path in VS Code
c file.txt                  # Open a file in VS Code
c --new-window              # Pass VS Code flags directly
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
1. Paths from `config.json` → `projects.server_paths`
2. Default paths: `java/serverJava`, `serverJava`, `backend`, `server`

**Web component (`w`, `web`):**
1. Paths from `config.json` → `projects.web_paths`
2. Default paths: `webapp`, `webApp`, `web`, `frontend`, `client`

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

#3 !!! [bug] Fix authentication system     (red)
#1 !! [feature] Implement user dashboard   (yellow)
#2 ! Complete documentation               (blue)
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
| `1` | ! | Low priority (blue) |
| `2` | !! | Medium priority (yellow) |
| `3` | !!! | High priority (red) |

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
| `n` | Add new todo |
| `e` | Edit selected todo |
| `x` or `Del` | Delete todo |
| `d` or `→` | Increase priority |
| `a` or `←` | Decrease priority |
| `o` | Toggle sort mode (also re-sorts) |
| `c` | Toggle completed view |
| `r` | Refresh from disk |
| `q` | Quit |

---

## `config` Command

Manage aliases-cli configuration.

### Syntax

```bash
aliases-cli config <subcommand> [args...]
```

### Subcommands

| Subcommand | Description |
|------------|-------------|
| `get <key>` | Get configuration value |
| `set <key> <value>` | Set configuration value |
| `list`, `ls` | List all configuration settings |
| `reset` | Reset configuration to defaults |
| `edit` | Open config file in editor |
| `path` | Show config file path |
| `sync setup <url> [method]` | Setup config sync |
| `sync pull` | Pull config from remote |
| `sync push` | Push config to remote |
| `sync status` | Show sync status |

### Examples

```bash
# View all settings
aliases-cli config list

# Get a specific value
aliases-cli config get general.editor

# Set a value
aliases-cli config set general.editor vim
aliases-cli config set code.reuse_window false
aliases-cli config set todo.default_priority 2

# Edit configuration file
aliases-cli config edit

# Show config file location
aliases-cli config path

# Reset to defaults
aliases-cli config reset

# Setup config sync
aliases-cli config sync setup git@github.com:user/aliases-config.git git
aliases-cli config sync push
aliases-cli config sync pull
aliases-cli config sync status
```

### Configuration Categories

- `general.*` - General settings (editor, colors, verbosity)
- `code.*` - Code command settings
- `todo.*` - Todo command settings
- `env.*` - Environment command settings
- `sync.*` - Config sync settings
- `projects.*` - Project mappings and workspace settings

See [Configuration Reference](configuration.md) for complete details on all settings.

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
- `~/.config/aliases-cli/config.json` - Main configuration including project mappings
- `~/.config/aliases-cli/todos.json` - Todo data storage

See [Configuration Guide](../user-guide/configuration.md) for details.