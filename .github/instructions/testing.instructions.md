---
description: "Testing standards for aliases. Use when writing or modifying unit tests, creating test fixtures, or running tests. Covers pytest patterns, config isolation, CLI integration tests via CliRunner, and mocking."
applyTo: "tests/**"
---

# Testing Conventions for aliases

## Framework & Location

- **pytest** 8+ with **Click 8.4+ CliRunner**
- Unit tests: `tests/test_<module>.py`
- CLI integration tests: `tests/test_cli_<command>.py`
- Shared fixtures: `tests/conftest.py`
- Run: `uv run pytest`
- Coverage: `uv run pytest --cov=aliases`

## Test Module Responsibilities

| Test file | Type | What it covers |
|---|---|---|
| `test_config.py` | Unit | Singleton lifecycle, get/set (type coercion), save/reload, deep-merge, corrupt JSON |
| `test_project_mapper.py` | Unit | Project discovery, hidden dirs, ignore list, shortcuts, component detection, by-path lookup |
| `test_pwd_formatter.py` | Unit | Home substitution, env_var/literal rules, rule priority, ANSI codes, --no-color, --ps1 |
| `test_cli_config.py` | Integration | config get/set/list/reset/path/sync-status via CliRunner |
| `test_cli_env.py` | Integration | env command: exports, project detection, profile, port offset |
| `test_cli_pwd.py` | Integration | pwd command: no-color, ANSI, PS1 delimiters, replacement rules |
| `test_cli_secrets.py` | Integration | secrets set/get/list/delete/load with mocked keyring |
| `test_cli_code.py` | Integration | code/c command: project lookup, components, fallback, with mocked Popen |
| `test_cli_completion.py` | Integration | completion projects/components helpers |

## Config Isolation — REQUIRED for every test

`isolated_config` in `conftest.py` is `autouse=True` — it runs automatically before every test.

It does:
1. `Config.set_test_config_directory(tmp_path / "aliases")` — redirects to a temp dir
2. `yield Config.instance()` — test runs
3. `Config.reset()` — singleton cleared for the next test

**Never** touch `~/.config/aliases/` in tests. Never define a local `isolated_config` — use the one from conftest.

## Standard Fixture (conftest.py)

```python
@pytest.fixture(autouse=True)
def isolated_config(tmp_path: Path):
    Config.set_test_config_directory(tmp_path / "aliases")
    yield Config.instance()
    Config.reset()

@pytest.fixture
def runner() -> CliRunner:
    return CliRunner()

@pytest.fixture
def workspace(tmp_path: Path) -> Path:
    ws = tmp_path / "workspaces"
    ws.mkdir()
    return ws
```

Helper function (not a fixture): `from tests.conftest import make_project`

## Unit Test Pattern

```python
from aliases.config import Config
from aliases.project_mapper import ProjectMapper
from tests.conftest import make_project

def test_finds_project(workspace):
    make_project(workspace, "dispatch")
    cfg = Config.instance()
    cfg._data["projects"]["workspace_directories"] = [str(workspace)]
    mapper = ProjectMapper(cfg)
    project = mapper.find_project("dispatch")
    assert project is not None
    assert project.full_name == "dispatch"
```

## CLI Integration Test Pattern

```python
from aliases.main import cli

def test_config_get_default(runner):
    result = runner.invoke(cli, ["config", "get", "general.editor"])
    assert result.exit_code == 0
    assert result.stdout.strip() == "code"
```

### Output Properties (Click 8.4+)

| Property | Content |
|----------|---------|
| `result.stdout` | Standard output only |
| `result.stderr` | Standard error only |
| `result.output` | Mixed stdout + stderr |

Always use `result.stdout` for output checks, `result.stderr` for error/warning checks.

### ANSI Colour Tests

Pass `color=True` to `invoke()` to preserve escape codes:

```python
def test_has_ansi(runner):
    result = runner.invoke(cli, ["pwd"], color=True)
    assert "\x1b[" in result.stdout
```

## Mocking Patterns

### Mock subprocess.Popen (code command)

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
def test_env_command(runner, workspace, monkeypatch):
    monkeypatch.setenv("PWD", str(workspace / "myproject"))
    result = runner.invoke(cli, ["env"])
    assert "export PROJECT_NAME=" in result.stdout
```

## Rules

- `isolated_config` is autouse — do NOT define it locally in test modules
- Always check `result.exit_code == 0` for success cases
- Never launch real VS Code → patch `subprocess.Popen`
- Never touch real keychain → patch `keyring.*`
- Never call `subprocess.run(["aliases", ...])` in tests → use `runner.invoke(cli, [...])`
- Group related tests in classes
- Use plain `assert` — pytest shows better diffs than `assertEqual`

## Running Tests

```bash
uv run pytest               # all tests
uv run pytest -v            # verbose
uv run pytest -x            # stop on first failure
uv run pytest -k cli_config # filter by name
uv run pytest --cov=aliases --cov-report=term-missing
```
