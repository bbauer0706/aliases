# Installation

## Requirements

- Python 3.12 or newer
- [uv](https://docs.astral.sh/uv/) — the recommended Python package manager
- bash 4.0+ for shell integration (zsh is not currently supported)

## Install

```bash
uv tool install git+https://github.com/bbauer0706/aliases
```

## First-Time Setup

Run once after installing:

```bash
aliases setup
```

What it does:

1. Creates `~/.config/aliases/` with the default `config.json`
2. Copies shell integration, bash aliases, and completion files there
3. Creates (or updates) `~/.bash_aliases` that sources all those files
4. Adds `source ~/.bash_aliases` to `~/.bashrc` if missing

Then restart your shell or:

```bash
source ~/.bash_aliases
```

## Headless / CI Environments

Systems without a GUI keychain (servers, CI) need the fallback backend:

```bash
uv tool install git+https://github.com/bbauer0706/aliases --extra keyring-fallback
```

This adds `keyrings.alt` which stores secrets in an encrypted file instead of
the OS keychain.

## Updating

```bash
aliases update           # checks GitHub, reinstalls, refreshes shell files
```

`aliases update` runs `aliases setup --update` for you afterwards, so the
bundled shell files never lag behind the installed version.

To refresh the shell files on their own (e.g. after `uv tool upgrade aliases`):

```bash
aliases setup --update
```

`--update` overwrites the files inside `~/.config/aliases/` and regenerates
`~/.bash_aliases` (backing up the old one to `~/.bash_aliases.bak` if it
changed). It does not touch `~/.bashrc` and does not prompt to install fzf or
ripgrep.

## Uninstalling

```bash
uv tool uninstall aliases
rm -rf ~/.config/aliases
# Remove the source line from ~/.bash_aliases and ~/.bashrc manually
```

## Development Install

```bash
git clone https://github.com/bbauer0706/aliases
cd aliases
uv sync --group dev
uv run aliases --version
```
