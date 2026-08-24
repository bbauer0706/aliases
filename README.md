# aliases

Developer workspace management for your shell. Quick-open projects in VS Code, set up project environment variables, manage secrets via the OS keychain, and format your shell prompt – all with one tool.

[![Python 3.12+](https://img.shields.io/badge/python-3.12%2B-blue.svg)](https://www.python.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## Installation

```bash
uv tool install git+https://github.com/bbauer0706/aliases
```

Then run the one-time setup:

```bash
aliases setup
```

This creates `~/.config/aliases/`, wires up `~/.bash_aliases`, and adds a `source` line to `~/.bashrc`. Restart your shell (or `source ~/.bash_aliases`) and you are done.

### Headless / CI environments (no system keychain)

```bash
uv tool install git+https://github.com/bbauer0706/aliases --extra keyring-fallback
```

---

## Quick Reference

```
aliases code   [PROJECT] [s|w|sw|[sw]]   # open VS Code
aliases env    [-e ENV] [-p PORT] [-n]   # export project env vars (eval)
aliases config get|set|list|reset|edit   # manage config
aliases secrets set|get|list|load|delete # manage keychain secrets
aliases pwd    [--ps1] [--no-color]      # formatted working directory
aliases setup  [--update] [--force]      # (re)install shell integration
```

The bash alias `c` maps to `aliases code`:

```bash
c dispatch          # open project
c dispatch s        # open server component
c dispatch[sw]      # open server + web
c ..                # fallback to: code ..
```

Shell helpers are sourced from `~/.bash_aliases` – including [`glab`](#glab-shortcuts)
and [`gh`](#gh-shortcuts) shortcuts for working across a GitLab group or your GitHub account.

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
aliases config get general.editor
aliases config set general.editor vim
aliases config list
aliases config reset
aliases config edit
aliases config path
aliases config sync setup git@github.com:user/config-repo.git
aliases config sync pull
aliases config sync push
aliases config sync status
```

### `secrets`

Secrets are stored in the OS keychain (GNOME Keyring, macOS Keychain, Windows Credential Manager).

```bash
aliases secrets set MY_TOKEN          # prompts securely
aliases secrets get MY_TOKEN
aliases secrets list
aliases secrets delete MY_TOKEN
secrets_load                              # eval all secrets into shell
secrets_load MY_TOKEN DB_PASS            # eval specific secrets
```

### `pwd`

Formats `$PWD` using path-replacement rules from config.

```bash
aliases pwd             # formatted path
aliases pwd --ps1       # readline-safe (for PS1)
aliases pwd --no-color  # no ANSI codes
```

### `setup`

```bash
aliases setup           # first-time setup
aliases setup --update  # refresh shell files + ~/.bash_aliases
aliases setup --force   # overwrite without prompting
```

### `update`

```bash
aliases update          # reinstall if newer, then refresh shell files
aliases update --check  # check only
aliases update --force  # reinstall regardless of version
```

---

## `glab` shortcuts

Sourced from `~/.config/aliases/bash_aliases/glab.ali.sh`. Silently inactive when
[`glab`](https://gitlab.com/gitlab-org/cli) is not installed.

`glab` already handles merge requests and pipelines well; the gap is *finding* a
repo, since `glab repo clone` needs the full nested path. `glc` closes it with an
fzf picker over a cached list of every repo in the group.

| Command | Does |
|---------|------|
| `glc [query]` | Pick a repo from the group, clone it into your workspace directory, `cd` there |
| `glc -r` | Same, but refresh the repo cache first |
| `glmr [flags]` | Open an MR from the current branch – fills from commits, squashes, deletes the source branch. Refuses to run on `main`/`master`. Extra flags pass through (`glmr --draft`) |
| `glco` | Pick an open MR and check it out |
| `glmm` | Merge the current MR |
| `glo` | Open the current MR – or the repo – in the browser |
| `glci` | Live pipeline status for the current branch |
| `glciv` | Pipeline job TUI (trace, retry, cancel) |
| `glcir` | Retry a job |

Repos are cloned flat as `<workspace-dir>/<repo-name>`, so `c <repo-name>` and
tab-completion pick them up right away.

The group defaults to `evotess`; override it per shell with `GLAB_GROUP`:

```bash
GLAB_GROUP=othergroup glc
```

The repo list is cached in `~/.cache/aliases/glab-repos-<group>.txt` for 24 hours.

---

## `gh` shortcuts

The GitHub twin of the above, sourced from
`~/.config/aliases/bash_aliases/gh.ali.sh`. Scoped to your own repositories.

| Command | Does |
|---------|------|
| `ghc [query]` | Pick one of your repos, clone it into your workspace directory, `cd` there |
| `ghpr [flags]` | Open a PR from the current branch – pushes it first, fills from commits. Refuses to run on `main`/`master`. Extra flags pass through (`ghpr --draft`) |
| `ghco` | Pick an open PR and check it out |
| `ghm` | Squash-merge the current PR and delete the branch |
| `gho` | Open the current PR – or the repo – in the browser |
| `ghci` | Watch the checks for the current branch's PR |
| `ghciv` | Inspect a workflow run |
| `ghcir` | Rerun the failed jobs |

No repo cache here, unlike `glc`: one account is a single fast call.

List someone else's repositories with `GH_OWNER`:

```bash
GH_OWNER=someorg ghc
```

---

## Configuration

Config file: `~/.config/aliases/config.json`

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
- optional: [`fzf`](https://github.com/junegunn/fzf) for the pickers, [`glab`](https://gitlab.com/gitlab-org/cli) for the `gl*` commands, [`gh`](https://cli.github.com/) for the `gh*` commands

## Development

```bash
git clone https://github.com/bbauer0706/aliases
cd aliases
uv sync --extra dev
uv run pytest
```
