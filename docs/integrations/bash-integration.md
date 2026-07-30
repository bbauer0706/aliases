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

Installs a custom `PS1` on source. Provides:

| Function | Description |
|----------|-------------|
| `_aliases_update_prompt` | Hooked into `PROMPT_COMMAND`; rebuilds `PS1` on `cd` |

`aliases pwd --full-prompt --ps1` runs only when `$PWD` changes; every other
Enter press reuses the `PS1` already in place, so there is no subprocess per
prompt.

### PS1 Safety

`aliases pwd --ps1` wraps ANSI codes in `\001...\002` (readline
non-printing delimiters). Without these, bash miscounts the line length and
tab-completion / line editing breaks.

`PS1` is assigned its finished value and is deliberately **not** exported, and
holds no `${...}` references. Tools that snapshot the environment — VS Code's
Java launcher, for one — otherwise inherit a `PS1` referring to a shell
variable they cannot resolve, and fail trying to expand it.

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

### Disabling

| Method | Scope |
|--------|-------|
| `ALIASES_NO_PROMPT=1` | Per shell. Checked before any `aliases` subprocess runs, so it also removes the startup cost. Set it before sourcing — e.g. via `terminal.integrated.env.linux` in VS Code. |
| `aliases config set prompt.enabled false` | Every shell. |

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
