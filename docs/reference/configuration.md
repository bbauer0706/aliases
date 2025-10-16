# Configuration Reference

Complete reference for aliases-cli configuration system.

## Configuration Location

All configuration is stored in:
```
~/.config/aliases-cli/
├── config.json          # Main configuration (includes project settings)
├── todos.json          # Todo data
└── cache/              # Cache directory
    └── sync/           # Config sync cache
```

## Managing Configuration

### Using the Config Command

```bash
# View all settings
aliases-cli config list

# Get a specific value
aliases-cli config get general.editor

# Set a value
aliases-cli config set general.editor vim

# Edit config file directly
aliases-cli config edit

# Show config file path
aliases-cli config path

# Reset to defaults
aliases-cli config reset
```

## Configuration Categories

### General Settings (`general.*`)

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `editor` | string | `code` | Default editor for opening files |
| `terminal_colors` | boolean | `true` | Enable/disable colored output |
| `verbosity` | string | `normal` | Verbosity level: `quiet`, `normal`, `verbose` |
| `confirm_destructive_actions` | boolean | `true` | Require confirmation for delete/remove operations |

**Examples:**
```bash
aliases-cli config set general.editor vim
aliases-cli config set general.verbosity verbose
aliases-cli config set general.terminal_colors false
```

---

### Code Command Settings (`code.*`)

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `vscode_flags` | array | `[]` | Default VS Code flags (e.g., `--new-window`) |
| `reuse_window` | boolean | `true` | Reuse existing VS Code window |
| `fallback_behavior` | string | `auto` | Fallback behavior: `always`, `never`, `auto` |
| `preferred_component` | string | `server` | Default component when ambiguous: `server`, `web`, `ask` |

**Examples:**
```bash
# Add VS Code flags
aliases-cli config edit  # Then add: "vscode_flags": ["--new-window"]

# Change fallback behavior
aliases-cli config set code.fallback_behavior never

# Set preferred component
aliases-cli config set code.preferred_component web
```

**Fallback Behavior Options:**
- `always`: Always fallback to `code` command if no project matches
- `never`: Never fallback, show error message instead
- `auto` (default): Smart fallback - fallback for paths and flags, error for unknown project names

---

### Todo Settings (`todo.*`)

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `default_priority` | integer | `0` | Default priority for new todos (0-3) |
| `default_sort` | string | `priority` | Default sort order: `priority`, `created`, `category`, `alphabetical` |
| `show_completed` | boolean | `false` | Include completed todos in list by default |
| `auto_categorize` | boolean | `false` | Enable smart auto-categorization of todos |

**Examples:**
```bash
# Set default priority for new todos
aliases-cli config set todo.default_priority 2

# Change default sort order
aliases-cli config set todo.default_sort created

# Show completed todos by default
aliases-cli config set todo.show_completed true
```

**Priority Levels:**
- `0`: No priority
- `1`: Low priority (🟢)
- `2`: Medium priority (🟡)
- `3`: High priority (🔴)

---

### Environment Settings (`env.*`)

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `base_port` | integer | `3000` | Starting port for projects |
| `port_offset` | integer | `100` | Port increment between projects |
| `default_env` | string | `dev` | Default environment: `dev`, `staging`, `prod` |

**Examples:**
```bash
# Change base port
aliases-cli config set env.base_port 8000

# Set port offset
aliases-cli config set env.port_offset 50

# Change default environment
aliases-cli config set env.default_env prod
```

**How Port Assignment Works:**
```
Project 1: base_port (3000)
Project 2: base_port + port_offset (3100)
Project 3: base_port + (port_offset * 2) (3200)
...
```

---

### Sync Settings (`sync.*`)

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `enabled` | boolean | `false` | Enable/disable config sync |
| `config_file_url` | string | `""` | HTTP URL to remote config.json file |
| `todo_file_url` | string | `""` | HTTP URL to remote todos.json file (optional) |
| `auto_sync.enabled` | boolean | `false` | Automatically sync on startup |
| `auto_sync.interval` | integer | `86400` | Seconds between automatic syncs (default: 24 hours) |
| `last_sync` | integer | `0` | Unix timestamp of last sync (auto-managed) |

**Examples:**
```bash
# Setup sync with HTTP URLs
aliases-cli config sync setup https://example.com/config.json https://example.com/todos.json

# Or just config file (todos optional)
aliases-cli config sync setup https://raw.githubusercontent.com/user/repo/main/config.json

# Pull latest config
aliases-cli config sync pull

# Check sync status
aliases-cli config sync status

# Enable auto-sync via JSON edit
aliases-cli config edit
# Then modify:
#   "auto_sync": {
#     "enabled": true,
#     "interval": 3600
#   }
```

**Simplified HTTP-Only Model:**

The sync feature now uses a simple HTTP fetch model for read-only config distribution. This is ideal for:
- Sharing team configurations via Git repositories (using raw.githubusercontent.com)
- Distributing configs from a web server
- Simple, reliable config synchronization without complex setup

**Migration from Old Format:**

If you have an old config with `remote_url` and `method` fields, they will be automatically migrated to the new format on next startup.

**See Also:** [Config Sync Guide](../user-guide/config-sync.md)

---

### Projects Settings (`projects.*`)

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `workspace_directories` | array | `[\"~/workspaces\"]` | Base directories to scan for projects (supports multiple sources) |
| `shortcuts` | object | `{}` | Project name shortcuts/aliases |
| `server_paths` | object | `{}` | Custom server component paths per project |
| `web_paths` | object | `{}` | Custom web component paths per project |
| `ignore` | array | `[]` | Directory names to ignore when scanning workspace |
| `default_paths.server` | array | `[\"java/serverJava\",\"serverJava\",\"backend\",\"server\"]` | Default server paths to search |
| `default_paths.web` | array | `[\"webapp\",\"webApp\",\"web\",\"frontend\",\"client\"]` | Default web paths to search |

**Examples:**
```bash
# View current project mappings
aliases-cli config get projects.shortcuts
aliases-cli config get projects.server_paths

# Add multiple workspace directories
aliases-cli config edit  # Then modify: "workspace_directories": ["~/workspaces", "~/dev/projects", "/opt/projects"]

# Ignore specific directories in workspace
aliases-cli config edit  # Then add to projects: "ignore": ["node_modules", "temp", ".git"]
```

**Multiple Workspace Sources:**
The `workspace_directories` array allows you to discover projects from multiple locations. All directories will be scanned and projects from all sources will be available:

```json
{
  "projects": {
    "workspace_directories": [
      "~/workspaces",
      "~/dev/personal-projects",
      "/mnt/shared/team-projects"
    ]
  }
}
```

**Ignore Patterns:**
The `ignore` array specifies directory names to exclude when scanning the workspace directory. This is useful for:
- Ignoring build artifacts or temporary directories
- Excluding symlinks or special directories
- Filtering out non-project directories

Example:
```json
{
  "projects": {
    "ignore": ["node_modules", "temp", "build", ".cache"]
  }
}
```

**Configuration via JSON:**
```json
{
  "projects": {
    "workspace_directories": ["~/workspaces", "~/dev/projects"],
    "shortcuts": {
      "urm20": "urm",
      "dispatch20": "dip"
    },
    "server_paths": {
      "urm20": "serverJava"
    },
    "web_paths": {
      "urm20": "urm2"
    },
    "ignore": [],
    "default_paths": {
      "server": ["java/serverJava", "serverJava", "backend", "server"],
      "web": ["webapp", "webApp", "web", "frontend", "client"]
    }
  }
}
```

**See Also:** [Commands Reference - Code Command](commands.md#code-command)

---

## Example Configuration File

Here's a complete example `config.json`:

```json
{
  "general": {
    "editor": "code",
    "terminal_colors": true,
    "verbosity": "normal",
    "confirm_destructive_actions": true
  },
  "code": {
    "vscode_flags": ["--reuse-window"],
    "reuse_window": true,
    "fallback_behavior": "auto",
    "preferred_component": "server"
  },
  "todo": {
    "default_priority": 1,
    "default_sort": "priority",
    "show_completed": false,
    "auto_categorize": false
  },
  "env": {
    "base_port": 3000,
    "port_offset": 100,
    "default_env": "dev"
  },
  "sync": {
    "enabled": false,
    "auto_sync": {
      "enabled": false,
      "interval": 86400
    },
    "last_sync": 0,
    "config_file_url": "",
    "todo_file_url": ""
  },
  "projects": {
    "workspace_directories": ["~/workspaces"],
    "shortcuts": {
      "urm20": "urm",
      "dispatch20": "dip"
    },
    "server_paths": {
      "urm20": "serverJava"
    },
    "web_paths": {
      "urm20": "urm2"
    },
    "ignore": [],
    "default_paths": {
      "server": ["java/serverJava", "serverJava", "backend", "server"],
      "web": ["webapp", "webApp", "web", "frontend", "client"]
    }
  }
}
```

## Advanced Usage

### Programmatic Access

In your bash scripts, you can read config values:

```bash
# Get editor setting
EDITOR=$(aliases-cli config get general.editor)

# Get base port
BASE_PORT=$(aliases-cli config get env.base_port)
```

### Configuration Migration

When upgrading aliases-cli, your configuration is automatically preserved. New settings are added with default values.

### Backup Configuration

```bash
# Backup
cp ~/.config/aliases-cli/config.json ~/.config/aliases-cli/config.json.backup

# Restore
cp ~/.config/aliases-cli/config.json.backup ~/.config/aliases-cli/config.json
```

### Reset Specific Categories

To reset a specific category, edit the config file and remove that section. It will be recreated with defaults on next run:

```bash
aliases-cli config edit
# Delete the "todo" section
# Save and exit

# Rerun any command to regenerate defaults
aliases-cli config list
```

## Troubleshooting

### Config File Not Found

If the config file is missing, it will be automatically created with defaults on first run.

### Invalid JSON

If your config file contains invalid JSON:

1. Check syntax: `python3 -m json.tool ~/.config/aliases-cli/config.json`
2. Fix errors or delete the file to regenerate defaults
3. Restore from backup if available

### Permission Issues

Ensure the config directory has proper permissions:
```bash
chmod 755 ~/.config/aliases-cli
chmod 644 ~/.config/aliases-cli/config.json
```

## See Also

- [Commands Reference](commands.md)
- [User Guide](../user-guide/README.md)
- [Project Mappings](projects.md)
