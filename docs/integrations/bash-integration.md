# Bash Integration

This directory contains bash shell integration scripts that provide wrapper functions around the C++ `aliases-cli` tool.

## Files

### `project-env.sh`

Provides bash wrapper functions for project environment management:

- **`project_env()`** - Sets up project environment variables by calling the C++ tool and eval'ing its output
- **`show_env()`** - Displays current environment variables using the C++ tool
- **`refresh_project_env()`** - Legacy compatibility function
- **`show_env_vars()`** - Legacy compatibility alias for `show_env()`
- **Legacy aliases** - `fix_env`, `fix_project`, `project_fix`
- **`auto_setup_new_terminal()`** - Automatically sets up environment for new terminals in workspace directories

### `prompt.sh`

Provides a custom `PS1` that replaces long path prefixes with short, optionally-coloured labels configured in `~/.config/aliases-cli/config.json`.

- **`aliases_setup_prompt()`** - Installs the custom PS1 into the current shell
- **`_aliases_prompt_pwd()`** - Internal helper called by PS1; invokes `aliases-cli pwd --ps1`

Both scripts are **automatically sourced** by the install script — no manual setup is needed.

## Why Bash Integration?

While the core `aliases-cli` tool is implemented in C++ for performance, some features require bash integration because:

1. **Environment variables** set by a child process (C++ executable) don't affect the parent shell
2. **The C++ tool outputs shell commands** that must be `eval`ed in the current shell context
3. **PS1 / prompt** must be set in the current shell — a subprocess cannot change it
4. **Shell-specific features** like auto-completion and environment management are best handled by the shell itself

## Prompt path replacement

The `aliases-cli pwd` command formats the current directory according to rules in the `prompt.path_replacements` config array. Each rule replaces a path prefix with a short label. Rules are evaluated in order; the first match wins and the rest are skipped.

### Rule fields

| Field | Required | Description |
|-------|----------|-------------|
| `env_var` | XOR with `path` | Env variable whose *runtime value* is used as the prefix |
| `path` | XOR with `env_var` | Literal path prefix (supports leading `~` for `$HOME`) |
| `label` | yes | Text to show instead of the matched prefix |
| `color` | no (default `bold_cyan`) | ANSI color name for the label |

Exactly one of `env_var` or `path` must be set per rule. Rules with both or neither set are silently ignored.

### Available colors

`black`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`  
All also available with `bold_` prefix, e.g. `bold_cyan`.

### Example configuration

```json
"prompt": {
  "enabled": true,
  "path_replacements": [
    {
      "env_var": "INSTROOT",
      "label": "INSTROOT",
      "color": "bold_cyan"
    },
    {
      "path": "/opt/company/platform",
      "label": "PLATFORM",
      "color": "bold_yellow"
    },
    {
      "path": "~/projects/internal",
      "label": "internal",
      "color": "bold_magenta"
    }
  ]
}
```

With `$INSTROOT=/srv/data/release` and the current directory `/srv/data/release/modules/auth`, the prompt shows:

```
bbauer@host:INSTROOT/modules/auth$
```

### CLI flags

```bash
aliases-cli pwd              # formatted path with color
aliases-cli pwd --no-color   # formatted path, no ANSI codes
aliases-cli pwd --ps1        # formatted path with PS1-safe ANSI wrapping (\001...\002)
```

## Usage

```bash
# Basic usage
project_env                    # Setup environment for current project
project_env -p 3000           # Setup with custom port
show_env                      # Display current environment variables

# Legacy compatibility
fix_env                       # Same as refresh_project_env
```

## Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌──────────────────┐
│   Bash Shell    │    │  Bash Wrapper    │    │  C++ aliases-cli │
│                 │    │                  │    │                  │
│ project_env()   │───▶│ Calls C++ tool   │───▶│ Generates export │
│                 │    │ Evals output     │◀───│ statements       │
│ Environment     │◀───│ Sets variables   │    │ Returns success  │
│ Variables Set   │    │                  │    │                  │
└─────────────────┘    └──────────────────┘    └──────────────────┘

┌─────────────────┐    ┌──────────────────┐    ┌──────────────────┐
│   Bash PS1      │    │  prompt.sh       │    │  C++ aliases-cli │
│                 │    │                  │    │                  │
│ $PS1 evaluated  │───▶│ _aliases_prompt  │───▶│ aliases-cli pwd  │
│ on each prompt  │    │ _pwd()           │◀───│ --ps1            │
│ Formatted path  │◀───│ Returns label    │    │ Applies config   │
│ shown           │    │                  │    │ rules            │
└─────────────────┘    └──────────────────┘    └──────────────────┘
```

This design provides:
- **Fast C++ tool** for logic and computation
- **Bash integration** for shell environment management
- **Seamless user experience** with environment variables properly set
