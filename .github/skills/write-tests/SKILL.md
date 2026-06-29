---
name: write-tests
description: "Write or extend tests for aliases. Use when asked to add tests for a command, fix a failing test, or improve coverage. Covers both unit tests (logic) and CLI integration tests (end-to-end via CliRunner). Handles isolation, mocking, and test structure."
argument-hint: "Describe what to test: command name, function, scenario, or 'full coverage for <module>'"
---

# Write Tests Skill

Adds unit tests and/or CLI integration tests to aliases following established patterns.

## When to Use

- Adding tests for a new or existing command
- Improving coverage for a core module
- Fixing a failing test
- Generating the test suite for a new feature after it's implemented

## Test Types

### Unit Tests

Test individual Python functions or classes directly, without going through the CLI.

- Location: `tests/test_<module>.py`
- Examples: `test_config.py`, `test_project_mapper.py`, `test_pwd_formatter.py`
- Use when: testing business logic, edge cases, pure functions

```python
from aliases.config import Config

def test_set_int(isolated_config):
    cfg = Config.instance()
    cfg.set("env.base_port", "4000")
    assert cfg.get("env.base_port") == 4000
```

### CLI Integration Tests

Test the CLI end-to-end by invoking Click commands via `CliRunner`. Run in the same process — no subprocess, no real config dir.

- Location: `tests/test_cli_<command>.py`
- Examples: `test_cli_config.py`, `test_cli_secrets.py`, `test_cli_code.py`
- Use when: testing a full command including argument parsing, output format, exit codes

```python
from aliases.main import cli

def test_config_get_default(runner):
    result = runner.invoke(cli, ["config", "get", "general.editor"])
    assert result.exit_code == 0
    assert result.stdout.strip() == "code"
```

## Fixtures (from `tests/conftest.py`)

| Fixture | Scope | Description |
|---------|-------|-------------|
| `isolated_config` | function (autouse) | Sets Config to `tmp_path/aliases`; resets after test |
| `runner` | function | `CliRunner()` — stdout and stderr always separated (Click 8.4+) |
| `workspace` | function | Empty `tmp_path/workspaces/` directory |

`isolated_config` runs **automatically for every test**. Never skip it; never touch `~/.config/aliases/`.

Helper function (import directly): `from tests.conftest import make_project`

## Output Properties (Click 8.4+)

| Property | Content |
|----------|---------|
| `result.stdout` | Standard output only |
| `result.stderr` | Standard error only |
| `result.output` | Mixed stdout + stderr (as user sees in terminal) |

Always use `result.stdout` for output checks and `result.stderr` for error/warning checks.

## Mocking Patterns

### Mock subprocess (code command)

```python
from unittest.mock import patch

def test_opens_project(runner, workspace):
    with patch("aliases.commands.code_navigator.subprocess.Popen") as mock:
        result = runner.invoke(cli, ["code", "myproject"])
    assert mock.called
    assert "myproject" in " ".join(mock.call_args[0][0])
```

### Mock keyring (secrets command)

```python
from unittest.mock import patch

def test_get_secret(runner):
    with patch("keyring.get_password", return_value="supersecret"):
        result = runner.invoke(cli, ["secrets", "get", "MY_KEY"])
    assert "supersecret" in result.stdout
```

### Mock environment / PWD

```python
def test_env_in_project_dir(runner, workspace, monkeypatch):
    monkeypatch.setenv("PWD", str(workspace / "myproject"))
    result = runner.invoke(cli, ["env"])
    assert "export PROJECT_NAME=" in result.stdout
```

### Test ANSI colour output

Pass `color=True` to `runner.invoke()` to preserve ANSI codes in output:

```python
def test_has_ansi(runner):
    result = runner.invoke(cli, ["pwd"], color=True)
    assert "\x1b[" in result.stdout
```

## Procedure

### Phase 1 — Understand What to Test

1. Read the command/module.
2. Identify: happy paths, edge cases, error paths, exit codes, output format.
3. Check `tests/conftest.py` for available fixtures.

### Phase 2 — Write Tests

1. Create `tests/test_cli_<cmd>.py` for CLI integration tests.
2. Create `tests/test_<module>.py` for unit tests.
3. Group related tests in classes (`class TestConfigGet`, `class TestConfigSet`, etc.).
4. Use descriptive names: `test_<what>_<condition>`.

### Phase 3 — Verify

```bash
uv run pytest tests/test_cli_<cmd>.py -v   # new tests only
uv run pytest -q                            # all must still be green
```

## Rules

- **NEVER** touch `~/.config/aliases/` — `isolated_config` is autouse; it always runs
- **NEVER** actually launch VS Code — patch `subprocess.Popen`
- **NEVER** write to the real OS keychain — patch `keyring.*`
- Use `runner.invoke(cli, [...])` — not `subprocess.run(["aliases", ...])`
- Check `result.exit_code == 0` for success, `!= 0` for failure
- Use `result.stdout` for stdout checks, `result.stderr` for stderr checks
- Prefer plain `assert` over `assertEqual`
