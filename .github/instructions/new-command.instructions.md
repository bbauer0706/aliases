---
description: "Use when adding a new CLI subcommand, implementing a new feature block, or extending an existing command in aliases. Covers the full checklist: Click command, config keys, tests, bash integration, and docs."
---

# Adding a New Command to aliases

Follow every step in order.

## 1. Click Command — `aliases/commands/my_cmd.py`

```python
import click
from aliases.config import Config
from aliases.project_mapper import ProjectMapper


@click.command("my-cmd")
@click.argument("args", nargs=-1)
@click.pass_context
def my_command(ctx: click.Context, args: tuple[str, ...]) -> None:
    """Short description shown in --help."""
    config = Config.instance()
    mapper: ProjectMapper = ctx.obj["mapper"]
    # ...
    raise SystemExit(0)
```

Rules:
- One `@click.command` (or `@click.group`) per file.
- Print errors with `click.echo("...", err=True)`.
- Shell-evaluable output to stdout; status messages to stderr.
- Exit codes: `0` success, `1` runtime error, `2` bad usage.

## 2. Register in `aliases/main.py`

```python
from aliases.commands.my_cmd import my_command

cli.add_command(my_command)
```

Add in alphabetical order among the other `add_command` calls.

## 3. Add Config Keys (if needed) — `aliases/config.py`

Add to `DEFAULT_CONFIG`:

```python
DEFAULT_CONFIG = {
    ...
    "my_section": {
        "my_key": "default_value",
    },
}
```

## 4. Document Config Keys — `docs/reference/configuration.md`

Add a table entry for every new key under the correct section heading.

## 5. Tests — `tests/test_my_cmd.py`

```python
import pytest
from pathlib import Path
from aliases.config import Config


@pytest.fixture(autouse=True)
def isolated_config(tmp_path: Path):
    Config.set_test_config_directory(tmp_path / "aliases")
    yield
    Config.reset()


def test_my_cmd_basic():
    ...
```

See `.github/instructions/testing.instructions.md` for full patterns.

## 6. Bash Integration (if output needs `eval`)

If the command prints `export VAR='value';` lines:

1. Create `aliases/data/shell/my-cmd.sh` with a bash wrapper function.
2. Add the filename to the copy list in `aliases/commands/setup_cmd.py` (`_SHELL_FILES`).
3. Document in `docs/integrations/bash-integration.md`.

## 7. Update Docs

- `docs/reference/commands.md` — add full command reference entry.
- `docs/reference/configuration.md` — add any new config keys.
- `docs/integrations/bash-integration.md` — if bash wrapper added.

## Verification

```bash
uv run aliases my-cmd --help
uv run pytest tests/test_my_cmd.py -v
uv run pytest -q   # all green
```
