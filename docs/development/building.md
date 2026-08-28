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
uv run aliases --version
uv run aliases --help
uv run aliases config list
uv run aliases completion projects
```

## Tests

```bash
uv run pytest               # all tests
uv run pytest -v            # verbose
uv run pytest -k config     # match test names
uv run pytest --cov=aliases # with coverage
uv run ruff check .         # lint (same command as CI)
```

## Building the Wheel

```bash
uv build
# → dist/aliases-2.0.0-py3-none-any.whl
```

## Installing the Local Build

```bash
uv tool install dist/aliases-2.0.0-py3-none-any.whl --force-reinstall
```

Or stay in the dev venv:

```bash
uv sync      # re-install editable
uv run aliases setup
```

## Releasing

Push conventional commits to `main`. After tests and Ruff pass, GitHub Actions
runs Commitizen to calculate the next semantic version, update `CHANGELOG.md`,
commit the bump, tag it, and create a GitHub Release. Commits with no releasable
change (for example `docs:` or `chore:`) pass without creating a release.

For a local preview without changing files:

```bash
uv run cz bump --dry-run
```
