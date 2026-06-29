# Bash Integration

`aliases setup` installs three shell integration files into
`~/.config/aliases/shell/`. They are sourced by `~/.bash_aliases`.

---

## `shell/project-env.sh`

Provides:

| Function / Alias | Description |
|-----------------|-------------|
| `project_env [opts]` | Calls `aliases env`, evals output to export vars |
| `show_env` | Calls `aliases env --show` |
| `refresh_project_env` | Re-runs `project_env` and calls `show_env` |
| `fix_env`, `fix_project`, `project_fix` | Aliases for `refresh_project_env` |

### Why eval?

`aliases env` prints `export VAR='value';` lines to stdout.
A child process cannot modify the parent shell's environment directly —
eval bridges that gap.

### Auto-setup

Set `ALIASES_AUTO_SETUP_ENV=1` before sourcing to auto-run `project_env`
when opening a terminal inside a workspace directory.

---

## `shell/prompt.sh`

Provides:

| Function | Description |
|----------|-------------|
| `aliases_setup_prompt` | Installs a custom `PS1` |
| `_aliases_prompt_pwd` | Called by `PS1`; invokes `aliases pwd --ps1` |

### PS1 Safety

`aliases pwd --ps1` wraps ANSI codes in `\001...\002` (readline
non-printing delimiters). Without these, bash miscounts the line length and
tab-completion / line editing breaks.

### Path Replacement

The formatted path is driven by `prompt.path_replacements` rules in config.
Each rule replaces a path prefix with a short label.

```json
"path_replacements": [
  { "env_var": "INSTROOT", "label": "INSTROOT", "color": "bold_yellow" },
  { "path": "~/work",      "label": "work",      "color": "bold_cyan"   }
]
```

Rules are evaluated in order; the first match wins. With no match the path
is shown with `~` substituted for `$HOME`.

### Auto-enable

Set `ALIASES_AUTO_SETUP_PROMPT=1` before sourcing to call `aliases_setup_prompt`
automatically on source.

---

## `shell/secrets.sh`

Provides:

| Function / Alias | Description |
|-----------------|-------------|
| `secrets_load [names...]` | Evals `aliases secrets load` output |
| `sload` | Short alias for `secrets_load` |

Exports the requested (or all) secrets from the OS keychain into the current
shell session.

---

## Prompt Colors

Available color names for `prompt.user_host_color`,
`prompt.default_path_color`, and rule `color` fields:

`black` `red` `green` `yellow` `blue` `magenta` `cyan` `white`

All also available with `bold_` prefix: `bold_green`, `bold_yellow`, etc.
