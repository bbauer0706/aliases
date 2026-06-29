# Configuration Reference

Config file: `~/.config/aliases/config.json`

Edit directly with `aliases config edit`, or use get/set:

```bash
aliases config get general.editor
aliases config set general.editor vim
aliases config list
```

---

## `general`

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `general.editor` | string | `code` | Editor opened by `config edit` |
| `general.terminal_colors` | bool | `true` | Enable ANSI color output |
| `general.verbosity` | string | `normal` | `quiet` / `normal` / `verbose` |
| `general.confirm_destructive_actions` | bool | `true` | Prompt before reset/delete |

---

## `code`

Settings for the `code` / `c` command.

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `code.reuse_window` | bool | `true` | Pass `--reuse-window` to VS Code |
| `code.vscode_flags` | list | `[]` | Extra flags passed to `code` |
| `code.fallback_behavior` | string | `auto` | `always` / `never` / `auto` — when to fall back to plain `code` |
| `code.preferred_component` | string | `server` | Default component when ambiguous: `server` / `web` / `ask` |

---

## `env`

Settings for the `env` command.

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `env.base_port` | int | `3000` | Lowest port to consider when scanning |
| `env.port_offset` | int | `100` | Unused (offset is derived from project name hash) |
| `env.default_env` | string | `dev` | Default `-e` value |

---

## `sync`

Settings for `config sync`.

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `sync.enabled` | bool | `false` | Enable remote sync |
| `sync.remote_url` | string | `""` | Remote URL (git repo, rsync path, file path, or HTTP URL) |
| `sync.method` | string | `git` | `git` / `rsync` / `file` / `http` |
| `sync.auto_sync` | bool | `false` | Pull on startup when interval has elapsed |
| `sync.sync_interval` | int | `86400` | Seconds between auto-sync pulls |
| `sync.last_sync` | int | `0` | Unix timestamp of last successful sync (auto-managed) |

---

## `projects`

Project discovery settings.

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `projects.workspace_directories` | list | `["~/workspaces"]` | Directories to scan for projects |
| `projects.shortcuts` | object | `{}` | Map of `"display_name": "full_dir_name"` |
| `projects.server_paths` | object | `{}` | Override server component path per project: `"project": "rel/path"` |
| `projects.web_paths` | object | `{}` | Override web component path per project |
| `projects.ignore` | list | `[]` | Directory names to skip during scan |
| `projects.default_paths.server` | list | `["java/serverJava","serverJava","backend","server"]` | Default server subdir search order |
| `projects.default_paths.web` | list | `["webapp","webApp","web","frontend","client"]` | Default web subdir search order |

### Shortcuts Example

```json
"shortcuts": {
  "dis": "dispatch-backend",
  "ui":  "company-frontend"
}
```

Then `c dis` opens the `dispatch-backend` directory.

### Multiple Workspace Directories

```json
"workspace_directories": ["~/workspaces", "~/work/clients"]
```

---

## `prompt`

PS1 / prompt formatting settings.

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `prompt.enabled` | bool | `true` | Enable custom prompt |
| `prompt.user_host_color` | string | `bold_green` | ANSI color for `user@host` |
| `prompt.default_path_color` | string | `bold_blue` | Color when no replacement rule matches |
| `prompt.path_replacements` | list | `[]` | Ordered list of path replacement rules |

### Path Replacement Rules

Each rule in `path_replacements` must have exactly one of `env_var` or `path`,
plus a `label`. The first matching rule wins.

| Field | Required | Description |
|-------|----------|-------------|
| `env_var` | XOR `path` | Env variable whose runtime value is used as the path prefix |
| `path` | XOR `env_var` | Literal path prefix (supports leading `~`) |
| `label` | yes | Short text shown in place of the prefix |
| `color` | no | ANSI color name for the label (default: `bold_cyan`) |

Available colors: `black`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`,
`white` — all also with `bold_` prefix (e.g. `bold_cyan`).

### Example

```json
"prompt": {
  "enabled": true,
  "user_host_color": "bold_green",
  "default_path_color": "bold_blue",
  "path_replacements": [
    { "env_var": "INSTROOT",             "label": "INSTROOT",  "color": "bold_yellow" },
    { "path":    "/opt/company/platform","label": "PLATFORM",  "color": "bold_yellow" },
    { "path":    "~/projects/internal",  "label": "internal",  "color": "bold_magenta" }
  ]
}
```

With `$INSTROOT=/srv/release` and CWD `/srv/release/auth/src`, the prompt shows:

```
you@host:INSTROOT/auth/src$
```

---

## `secrets`

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `secrets.password_env_var` | string | `ALIASES_MASTER_PASSWORD` | Unused (kept for legacy compat; keyring handles auth) |

Secrets are stored in the OS keychain — no file path or master password config is needed.
