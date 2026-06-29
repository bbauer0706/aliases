---
description: "Python coding standards for aliases. Use when writing or modifying .py source files. Covers naming, error handling, imports, patterns, and style used in this codebase."
applyTo: "**/*.py"
---

# Python Coding Conventions

## Language Version

Python 3.12+. Use modern syntax: `match`, `str | None`, `X | Y` unions, `@dataclass`, `type` aliases.

## Naming

| Kind | Style | Example |
|------|-------|---------|
| Classes | `PascalCase` | `ProjectMapper` |
| Functions / methods | `snake_case` | `find_project()` |
| Private methods | `_snake_case` | `_deep_merge()` |
| Constants | `ALL_CAPS` | `SERVICE_NAME` |
| Module-level singletons | `_instance` private var + class accessor | `Config._instance` |

## Imports

Standard order (separated by blank lines):
1. stdlib (`os`, `sys`, `pathlib`, `json`, …)
2. third-party (`click`, `keyring`, `rich`, …)
3. project (`from aliases.config import Config`)

No wildcard imports.

## Error Handling

- Return `None` or a `bool` / `tuple` for recoverable failures. No bare `except:`.
- Raise only for programming errors (invalid arguments the caller controls).
- Print user-facing errors via `click.echo("...", err=True)`.
- Exit codes: `0` success, `1` runtime error, `2` bad usage.

## Click Commands

- Command functions decorated with `@click.command()` or `@click.group()`.
- Options use `snake_case` parameter names; Click converts `--some-flag` automatically.
- Pass `Config.instance()` and `ProjectMapper` as regular arguments (no global state in commands).
- Commands that output shell-evaluable text write to **stdout**; status messages to **stderr**.

## Config Access

- Always via `Config.instance()` — never instantiate `Config` directly in commands.
- Read: `config.get("section.key")` — returns `None` if missing.
- Write: `config.set("section.key", value)` then `config.save()`.
- Tests: `Config.set_test_config_directory(tmp_path)` + `Config.reset()` in fixtures.

## Patterns Used

- **Singleton** — `Config` uses `_instance` class variable with `instance()` accessor.
- **Dataclass** — `ProjectInfo` is a `@dataclass`.
- **`importlib.metadata`** — version string in `__init__.py`.
- **`importlib.resources`** — access bundled `data/` files via `files("aliases") / "data"`.

## File / Path Handling

Use `pathlib.Path` everywhere. No `os.path.join()`.

## Output / ANSI

- Use ANSI codes only when `config.get("general.terminal_colors")` is `True` (default).
- For PS1 wrapping use `\001...\002` around ANSI sequences (readline non-printing markers).
- `rich` is available for pretty tables — use it only in `config list`.

## No-Nos

- No `subprocess.shell=True` with user input.
- No `eval()` in Python code.
- No hardcoded paths to the binary — shell scripts call `aliases` from PATH.
- No `print()` in command code — use `click.echo()`.
