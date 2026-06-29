# Architecture

## Overview

aliases is a Click-based Python CLI. Modules are organized in a flat
dependency hierarchy:

```
commands/ → core modules → stdlib
```

Core modules do not import from commands. No circular dependencies.

## Package Layout

```
src/aliases/
├── __init__.py          # version via importlib.metadata
├── main.py              # Click group, entry point (aliases.main:main)
├── config.py            # Config singleton
├── project_mapper.py    # Project discovery + component detection
├── config_sync.py       # Remote config sync
├── pwd_formatter.py     # PS1 path formatting
├── process_utils.py     # subprocess helpers + port check
├── git_operations.py    # git CLI wrappers
├── commands/
│   ├── code_navigator.py  # `code` / `c`
│   ├── config_cmd.py      # `config` + sync subgroup
│   ├── project_env.py     # `env`
│   ├── secrets_cmd.py     # `secrets`
│   └── setup_cmd.py       # `setup`
└── data/                  # bundled shell / alias / completion files
    ├── shell/
    │   ├── project-env.sh
    │   ├── prompt.sh
    │   └── secrets.sh
    ├── bash_aliases/
    │   ├── basic.ali.sh
    │   ├── clear.ali.sh
    │   ├── git.ali.sh
    │   ├── maven.ali.sh
    │   ├── npm.ali.sh
    │   └── syncrotess.ali.sh
    ├── bash_completion/
    │   └── aliases-completion.sh
    └── config.template.json
```

## Core Modules

### `config.py` — Config Singleton

- `Config.instance()` returns the singleton (lazy-initialised on first call)
- Config dir defaults to `~/.config/aliases/`; overridden by
  `Config.set_test_config_directory(path)` for test isolation
- `config.get("section.key")` — dot-notation access, returns `None` if missing
- `config.set("section.key", "value")` — type-coerces to the existing stored type
  (bool, int, float, list, or string)
- On load: deep-merges the saved JSON with `DEFAULT_CONFIG` so missing keys
  always get defaults

### `project_mapper.py` — ProjectInfo + Discovery

- `ProjectMapper(config).discover_projects()` → `list[ProjectInfo]`
- Scans each directory in `projects.workspace_directories`
- Skips hidden dirs (`.git`, `.venv`, …) and entries in `projects.ignore`
- Applies shortcut map: `full_name → display_name`
- Auto-detects server/web components by trying default subdir names in order,
  then custom overrides from `projects.server_paths` / `projects.web_paths`
- `find_project(name)` — exact match then case-insensitive fallback
- `find_project_by_path(path)` — returns the project that contains the path

### `config_sync.py` — Remote Sync

Four methods share the same `setup/pull/push/status` interface:
- **git**: clone to `~/.config/aliases/cache/sync/config-repo`, pull/commit/push. The path of `config.json` within the repo is controlled by `sync.repo_config_path` (default `config.json`).
- **rsync**: `rsync -az` to/from remote path
- **file**: `shutil.copy2` to/from local/network path
- **http**: `urllib.request.urlopen` (read-only)

### `pwd_formatter.py` — Prompt Path

`format_pwd(config, pwd=None, *, no_color, ps1)` applies
`prompt.path_replacements` rules in order. `get_user_host_color` returns the
raw ANSI escape code (optionally wrapped for readline).

### `process_utils.py`

Thin wrapper: `execute(cmd, cwd) → (code, stdout, stderr)`.  
`is_port_available(port)` — TCP connect with 50 ms timeout.

## Shell Integration

`aliases setup` copies bundled files from `aliases/data/` to
`~/.config/aliases/` and generates `~/.bash_aliases`. The bash functions
in `shell/*.sh` call `aliases` (from PATH) and `eval` its output.

```
bash: project_env()
  → aliases env [opts]   (captures stdout)
  → eval "export VAR=value;"
```

```
bash: PS1 via _aliases_prompt_pwd()
  → aliases pwd --ps1    (stdout → inserted into PS1 string)
```

## Entry Point

`pyproject.toml`:

```toml
[project.scripts]
aliases = "aliases.main:main"
```

`main.py` initialises Config (singleton), then delegates to Click's group dispatcher.
