# Testing

aliases uses [pytest](https://pytest.org) for all tests.

## Quick Start

```bash
uv run pytest           # run all tests
uv run pytest -v        # verbose
uv run pytest -x        # stop on first failure
uv run pytest --cov=src # coverage report
```

## Test Layout

```
tests/
├── test_config.py         # Config singleton, get/set, save/reload, deep-merge
├── test_project_mapper.py # Project discovery, shortcuts, components, by-path lookup
└── test_pwd_formatter.py  # Path replacement rules, color codes, PS1 wrapping
```

## Writing Tests

### Config Isolation

Every test that touches `Config` must redirect it to a temp directory:

```python
import pytest
from pathlib import Path
from aliases.config import Config

@pytest.fixture(autouse=True)
def isolated_config(tmp_path: Path):
    Config.set_test_config_directory(tmp_path / "aliases")
    yield
    Config.reset()
```

Use `autouse=True` at the class or module level to avoid forgetting.

### Example Test

```python
from aliases.config import Config
from aliases.project_mapper import ProjectMapper

def test_finds_project(tmp_path, isolated_config):
    ws = tmp_path / "workspaces"
    (ws / "dispatch").mkdir(parents=True)

    cfg = Config.instance()
    cfg._data["projects"]["workspace_directories"] = [str(ws)]

    mapper = ProjectMapper(cfg)
    project = mapper.find_project("dispatch")
    assert project is not None
    assert project.full_name == "dispatch"
```

### Project Fixtures

```python
@pytest.fixture
def workspace(tmp_path):
    ws = tmp_path / "workspaces"
    ws.mkdir()
    return ws

def make_project(workspace, name, server=None, web=None):
    p = workspace / name
    p.mkdir()
    if server: (p / server).mkdir(parents=True)
    if web:    (p / web).mkdir(parents=True)
    return p
```

## What Each Module Tests

| File | Covers |
|------|--------|
| `test_config.py` | Defaults, get, set (type coercion), save/reload, corrupt JSON, deep merge |
| `test_project_mapper.py` | Discovery, hidden dir filtering, ignore list, shortcuts, server/web detection, find by path |
| `test_pwd_formatter.py` | Home replacement, env_var rules, literal path rules, rule priority, color codes, `--no-color`, `--ps1` wrapping |

## CI

```bash
# All that is needed in CI:
uv run pytest
```

Exit code is non-zero on any failure.
