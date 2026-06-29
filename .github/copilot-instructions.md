# aliases Project Guidelines

## What This Project Is

`aliases` is a Python CLI tool for developer workspace management. It handles
project discovery, environment setup, config sync, and shell prompt formatting
across multi-project workspaces.

Installed via uv: `uv tool install git+https://github.com/bbauer0706/aliases`

## Architecture

```
aliases/main.py              → Click group entry point; initialises Config, routes to commands
aliases/config.py            → Singleton config (JSON at ~/.config/aliases/config.json)
aliases/project_mapper.py    → Discovers projects via workspace dirs + shortcuts
aliases/config_sync.py       → Multi-method remote sync (git/rsync/file/http)
aliases/pwd_formatter.py     → Shell prompt path formatting with ANSI + PS1 wrapping
aliases/process_utils.py     → subprocess helpers + port availability check
aliases/git_operations.py    → git CLI wrappers
aliases/commands/
  code_navigator.py  → opens project in VS Code with component awareness
  config_cmd.py      → get/set/list/reset config keys + sync subgroup
  project_env.py     → exports project environment variables to shell (eval)
  secrets_cmd.py     → OS keychain secrets via keyring
  setup_cmd.py       → post-install shell wiring wizard
aliases/data/                → bundled shell / alias / completion files
```

**Dependency direction**: `commands/` → core modules. Core modules do not import from commands.

## Key Types

- `ProjectInfo` — `@dataclass` with `full_name`, `display_name`, `path`, `server_path`, `web_path`, `has_server_component`, `has_web_component`
- `Config` — singleton; `Config.instance()` / `Config.reset()` / `Config.set_test_config_directory(path)`
- `config.get("section.key")` → `Any | None`; `config.set("section.key", value)` type-coerces

## Configuration

- Singleton: `Config.instance()` — lazy-initialised on first call
- JSON at `~/.config/aliases/config.json`
- `_deep_merge(base, override)` ensures missing keys always get defaults
- Tests: `Config.set_test_config_directory(tmp_path)` + `Config.reset()` for full isolation

## Build & Test Commands

```bash
uv sync --group dev    # install dev dependencies
uv run aliases     # run from dev checkout
uv run pytest          # run all tests
uv run pytest -v       # verbose
uv run pytest --cov=aliases  # with coverage
uv build               # build wheel
uv tool install dist/*.whl --force-reinstall  # test the wheel
```

## Python Conventions

- **Version**: Python 3.12+. Use `match`, `str | None`, `dataclass`, `importlib.metadata`.
- **Naming**: Classes `PascalCase`, functions/variables `snake_case`, private `_prefixed`, constants `ALL_CAPS`
- **Error handling**: return `None` / bool / tuple for recoverable failures. No bare `except:`. Exit via `raise SystemExit(code)` or `sys.exit(code)`.
- **Imports**: stdlib → third-party → project. No wildcard imports.
- **Paths**: `pathlib.Path` everywhere. No `os.path.join()`.
- **Output**: `click.echo()` in commands. Shell-evaluable output to stdout; status messages to stderr.

## Adding a New Command

See `.github/instructions/new-command.instructions.md` for the full checklist. Summary:
1. Create `aliases/commands/my_cmd.py` with a `@click.command` or `@click.group`
2. Register with `cli.add_command(my_command)` in `aliases/main.py`
3. Add config keys to `DEFAULT_CONFIG` in `aliases/config.py` if needed
4. Write tests in `tests/test_my_cmd.py` using the `isolated_config` fixture
5. Update `docs/reference/commands.md` and `docs/reference/configuration.md`

## Testing Conventions

See `.github/instructions/testing.instructions.md`. All tests use pytest.
Config-touching tests must use `Config.set_test_config_directory(tmp_path)` + `Config.reset()`.
Never touch `~/.config/aliases/` in tests.

## Bash Integration

Commands that output shell-evaluable text (like `env`, `secrets load`) print
`export VAR='value';` to stdout. Bash wrappers in `aliases/data/shell/` use
`eval "$(aliases ...)"` to apply them in the current shell. `setup_cmd.py`
copies these to `~/.config/aliases/shell/`.

## Documentation Policy

Every code change must include a corresponding doc update:

- New command → add entry to `docs/reference/commands.md`
- New or changed config key → update `docs/reference/configuration.md`
- New bash integration file → update `docs/integrations/bash-integration.md`
- Architectural change → update `docs/development/architecture.md`

## Docs

- `docs/development/architecture.md` — full architecture detail
- `docs/development/building.md` — build system
- `docs/development/testing.md` — test strategy
- `docs/reference/configuration.md` — all config keys
- `docs/reference/commands.md` — full command reference
