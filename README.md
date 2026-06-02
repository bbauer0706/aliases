# aliases-cli

Developer workspace management for your shell. Quick-open projects in VS Code, set up project environment variables, manage secrets via the OS keychain, and format your shell prompt – all with one tool.

[![Python 3.12+](https://img.shields.io/badge/python-3.12%2B-blue.svg)](https://www.python.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## Installation

```bash
uv tool install git+https://github.com/bbauer0706/aliases
```

Then run the one-time setup:

```bash
aliases-cli setup
```

This creates `~/.config/aliases-cli/`, wires up `~/.bash_aliases`, and adds a `source` line to `~/.bashrc`. Restart your shell (or `source ~/.bash_aliases`) and you are done.

### Headless / CI environments (no system keychain)

```bash
uv tool install git+https://github.com/bbauer0706/aliases --extra keyring-fallback
```

---

## Quick Reference

```
aliases-cli code   [PROJECT] [s|w|sw|[sw]]   # open VS Code
aliases-cli env    [-e ENV] [-p PORT] [-n]   # export project env vars (eval)
aliases-cli config get|set|list|reset|edit   # manage config
aliases-cli secrets set|get|list|load|delete # manage keychain secrets
aliases-cli pwd    [--ps1] [--no-color]      # formatted working directory
aliases-cli setup  [--update] [--force]      # (re)install shell integration
```

The bash alias `c` maps to `aliases-cli code`:

```bash
c dispatch          # open project
c dispatch s        # open server component
c dispatch[sw]      # open server + web
c ..                # fallback to: code ..
```

---

## Commands

### `code` / `c`

Open projects in VS Code. Falls back to the plain `code` command when no project matches.

| Syntax | Meaning |
|--------|---------|
| `c` | Open home directory |
| `c <project>` | Open project root |
| `c <project> s` | Open server component |
| `c <project> w` | Open web component |
| `c <project>[sw]` | Open server and web |
| `c <project>sw` | Shorthand for server + web |
| `c proj1 proj2` | Open multiple projects |
| `c <path>` | Fallback: `code <path>` |

### `env`

Discovers the project from `$PWD` and outputs shell export statements.
Use via the bash function: `project_env [OPTIONS]` (wraps eval).

```bash
project_env            # discover + export
project_env -e prod    # production profile
project_env -p 9000    # start port scan at 9000
project_env -n         # no GQL port offset
show_env               # display current values
```

Exported variables: `PROJECT_NAME`, `PROFILE`, `WEBPORT`, `GQLPORT`, `GQLNUMBEROFMAXRETRIES`, `GQLINTROSPECTION`, `GQLTRANSFERMODE`.

### `config`

```bash
aliases-cli config get general.editor
aliases-cli config set general.editor vim
aliases-cli config list
aliases-cli config reset
aliases-cli config edit
aliases-cli config path
aliases-cli config sync setup git@github.com:user/config-repo.git
aliases-cli config sync pull
aliases-cli config sync push
aliases-cli config sync status
```

### `secrets`

Secrets are stored in the OS keychain (GNOME Keyring, macOS Keychain, Windows Credential Manager).

```bash
aliases-cli secrets set MY_TOKEN          # prompts securely
aliases-cli secrets get MY_TOKEN
aliases-cli secrets list
aliases-cli secrets delete MY_TOKEN
secrets_load                              # eval all secrets into shell
secrets_load MY_TOKEN DB_PASS            # eval specific secrets
```

### `pwd`

Formats `$PWD` using path-replacement rules from config.

```bash
aliases-cli pwd             # formatted path
aliases-cli pwd --ps1       # readline-safe (for PS1)
aliases-cli pwd --no-color  # no ANSI codes
```

### `setup`

```bash
aliases-cli setup           # first-time setup
aliases-cli setup --update  # update shell files only
aliases-cli setup --force   # overwrite without prompting
```

---

## Configuration

Config file: `~/.config/aliases-cli/config.json`

| Key | Default | Description |
|-----|---------|-------------|
| `general.editor` | `code` | Editor for `config edit` |
| `general.terminal_colors` | `true` | Enable ANSI colors |
| `projects.workspace_directories` | `["~/workspaces"]` | Directories to scan |
| `projects.shortcuts` | `{}` | `{"alias": "full-name"}` |
| `env.base_port` | `3000` | Starting port |
| `prompt.path_replacements` | `[]` | Path to label rules |

See [docs/reference/configuration.md](docs/reference/configuration.md) for all keys.

---

## Requirements

- Python 3.12+
- [uv](https://docs.astral.sh/uv/) (recommended) or pip
- bash 4.0+ for shell integration

## Development

```bash
git clone https://github.com/bbauer0706/aliases
cd aliases
uv sync --extra dev
uv run pytest
```
