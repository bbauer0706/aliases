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
| [`config`](#config-command) | - | Manage aliases-cli configuration |
| [`env`](#env-command) | `project_env` | Setup environment variables |
| [`secrets`](#secrets-command) | - | Encrypted env-var secrets manager |

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

# Git repository issues
$ uw project-with-conflicts
✗ project-with-conflicts: Merge conflicts detected
  Please resolve conflicts manually
```

## Configuration Files

Commands read configuration from:
- `~/.config/aliases-cli/config.json` - Main configuration including project mappings
- `~/.config/aliases-cli/secrets.enc` - Encrypted secrets store

---

## `secrets` Command

Manage encrypted environment-variable secrets. Secrets are stored with **AES-256-GCM** encryption, with the key derived from a master password via **PBKDF2-SHA256** (100 000 iterations). The store file is `chmod 0600`.

### Syntax

```bash
aliases-cli secrets <subcommand> [args]
```

### Subcommands

| Subcommand | Description |
|------------|-------------|
| `set <name> [value]` | Store a secret. If `value` is omitted, prompts securely (no echo) |
| `get <name>` | Print the decrypted value to stdout |
| `list` | List all secret names (sorted) |
| `delete <name>` | Remove a secret permanently (`remove`/`rm` also accepted) |
| `load [name...]` | Output `export NAME=VALUE` lines — pipe to `eval` |

### Master Password

The master password is resolved in this order:
1. The env var specified by `secrets.password_env_var` (default `$ALIASES_MASTER_PASSWORD`)
2. Interactive prompt on `/dev/tty` with echo disabled

For scripted or CI use, set `ALIASES_MASTER_PASSWORD` before calling the command.

### Examples

```bash
# Store secrets
aliases-cli secrets set MY_API_KEY              # prompts securely for value
aliases-cli secrets set DB_PASS mysecret        # inline value (avoid in scripts)
aliases-cli secrets set GITHUB_TOKEN            # prompts securely

# Retrieve
aliases-cli secrets get MY_API_KEY

# List all names
aliases-cli secrets list

# Remove
aliases-cli secrets delete MY_API_KEY

# Load all into current shell
eval "$(aliases-cli secrets load)"

# Load specific secrets
eval "$(aliases-cli secrets load MY_API_KEY DB_PASS)"

# Using the bash wrapper (after sourcing bash_integration/secrets.sh)
secrets_load
secrets_load MY_API_KEY DB_PASS
```

### Bash Integration

Source `bash_integration/secrets.sh` to get the `secrets_load` helper:

```bash
# In ~/.bashrc
source /path/to/aliases-cli/bash_integration/secrets.sh

# Then use:
secrets_load               # export all secrets
secrets_load MY_API_KEY    # export one secret
```

### Security Notes

- Secret **names** and **values** are both encrypted — the file reveals nothing without the password
- GCM authentication tag prevents silent tampering
- Wrong password → decryption fails immediately with an error (no partial data leakage)
- Store file is created `0600`; never synced by `aliases-cli config sync`
- Avoid embedding values as CLI arguments (`secrets set KEY value`) in shell history; prefer the interactive prompt

See [Configuration Guide](../user-guide/configuration.md) for details.