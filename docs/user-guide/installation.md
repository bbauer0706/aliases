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
uv tool upgrade aliases
aliases setup --update   # refresh bundled shell/alias files
```

`--update` only overwrites the files inside `~/.config/aliases/` — it
does not touch `~/.bash_aliases` or `~/.bashrc`.

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
