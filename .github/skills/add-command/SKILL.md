---
name: add-command
description: "Add a new CLI subcommand or major feature block to aliases-cli. Use when implementing a new command end-to-end: Click command, config keys, tests, build system updates, and bash integration. Handles the full pipeline including codebase exploration, design, implementation, and verification."
argument-hint: "Describe the new command: name, purpose, arguments, config keys needed"
---

# Add Command Skill

Implements a complete new CLI subcommand in aliases-cli, following established Python/Click patterns.

## When to Use

- Adding a new top-level CLI subcommand (`aliases-cli my-cmd ...`)
- Implementing a major feature block that spans command + config + tests
- Extending the project with a new core module

## Procedure

### Phase 1 — Understand the Request

1. Clarify (if not obvious): command name, purpose, CLI syntax, required config keys, whether it needs bash integration.
2. Study the closest existing command as a reference:
   - `aliases_cli/commands/secrets_cmd.py` — click group with keychain state
   - `aliases_cli/commands/config_cmd.py` — click group with subcommands
   - `aliases_cli/commands/project_env.py` — eval-output pattern
3. Read `aliases_cli/main.py` to understand registration.

### Phase 2 — Design

1. Define the Click command/group signature.
2. List config keys needed (section, key, type, default).
3. Decide if a bash wrapper is needed (does output need `eval`-ing?).
4. Identify which core modules will be used (`ProjectMapper`, `GitOperations`, `ProcessUtils`).

### Phase 3 — Implement (follow `.github/instructions/new-command.instructions.md`)

Execute each step of the checklist:
1. Create `aliases_cli/commands/<cmd>.py` with `@click.command` or `@click.group`
2. Register in `aliases_cli/main.py` with `cli.add_command(...)`
3. Add config keys to `DEFAULT_CONFIG` in `aliases_cli/config.py`
4. Create `tests/test_<cmd>.py` with `isolated_config` fixture

### Phase 4 — Verify

```bash
uv run aliases-cli my-cmd --help   # smoke test
uv run pytest tests/test_my_cmd.py -v
uv run pytest -q                   # all green
```

Fix any failures before proceeding.

### Phase 5 — Bash Integration & Docs

- Create `aliases_cli/data/shell/my-cmd.sh` if the command outputs shell-evaluable code.
- Add filename to `_SHELL_FILES` in `aliases_cli/commands/setup_cmd.py`.
- Update `docs/reference/commands.md` and `docs/reference/configuration.md`.

## Key Files to Read Before Starting

- [new-command instructions](../../.github/instructions/new-command.instructions.md)
- [aliases_cli/main.py](../../aliases_cli/main.py) — registration pattern
- [aliases_cli/commands/secrets_cmd.py](../../aliases_cli/commands/secrets_cmd.py) — complex reference
- [aliases_cli/commands/config_cmd.py](../../aliases_cli/commands/config_cmd.py) — subgroup reference
- [aliases_cli/config.py](../../aliases_cli/config.py) — DEFAULT_CONFIG pattern
- [pyproject.toml](../../pyproject.toml) — dependencies

## Patterns to Follow

- `Config.instance()` for config reads — never instantiate inline.
- `click.echo("...", err=True)` for errors.
- Shell-evaluable output → stdout; status messages → stderr.
- `pathlib.Path` everywhere.
- `isolated_config` fixture in every test module that touches `Config`.
