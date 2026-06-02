# Building & Development

## Prerequisites

- Python 3.12+
- [uv](https://docs.astral.sh/uv/)

## Setup

```bash
git clone https://github.com/bbauer0706/aliases
cd aliases
uv sync --group dev
```

## Running Locally

```bash
uv run aliases-cli --version
uv run aliases-cli --help
uv run aliases-cli config list
uv run aliases-cli completion projects
```

## Tests

```bash
uv run pytest               # all tests
uv run pytest -v            # verbose
uv run pytest -k config     # match test names
uv run pytest --cov=src     # with coverage
```

## Building the Wheel

```bash
uv build
# → dist/aliases_cli-2.0.0-py3-none-any.whl
```

## Installing the Local Build

```bash
uv tool install dist/aliases_cli-2.0.0-py3-none-any.whl --force-reinstall
```

Or stay in the dev venv:

```bash
uv sync      # re-install editable
uv run aliases-cli setup
```

## Releasing

1. Tag the commit: `git tag v2.x.y && git push --tags`
2. `uv build`
3. `uv publish` (or push to GitHub; uv tool install works directly from git)

The version string is read from the git tag at build time via `hatchling`.
