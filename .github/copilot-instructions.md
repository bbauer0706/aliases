# aliases-cli Project Guidelines

## What This Project Is

`aliases-cli` is a high-performance C++ CLI tool (50× faster than bash equivalents) for developer workspace management. It handles project discovery, environment setup, todo management, config sync, and shell prompt formatting across multi-project workspaces.

## Architecture

```
src/main.cpp               → CLI dispatcher; initializes Config + ProjectMapper, routes to commands
src/core/                  → Shared infrastructure (no command logic here)
  config.cpp               → Singleton config (JSON at ~/.config/aliases-cli/config.json)
  project_mapper.cpp       → Discovers projects via workspace dirs + shortcuts (PIMPL pattern)
  config_sync.cpp          → Multi-method remote sync (git/rsync/file/http)
  file_utils.cpp           → Static path/filesystem utilities
  git_operations.cpp       → Static git CLI wrappers via ProcessUtils
  process_utils.cpp        → Process execution (sync + async via popen/future)
  common.cpp               → String utils + ANSI color constants
  pwd_formatter.cpp        → Shell prompt path formatting with ANSI + PS1 wrapping
src/commands/              → One class per top-level CLI subcommand
  code_navigator.cpp       → Opens project in VS Code with component awareness
  config_cmd.cpp           → get/set/list/reset config keys
  project_env.cpp          → Exports project environment variables to shell
  todo.cpp                 → TodoManager: CRUD + TUI + sync for todo items
```

**Dependency Direction**: commands → core. Core modules do not depend on commands.

## Key Types (include/aliases/common.h)

- `Result<T>` — generic success/error container; use `Result<T>::success_with(val)` and `Result<T>::error(msg)`. Never throw for recoverable errors.
- `ProjectInfo` — discovered project with name, path, optional server/web component paths, `ComponentType` enum (`MAIN`/`SERVER`/`WEB`)
- `StringMap` / `StringVector` — `unordered_map<string,string>` / `vector<string>` aliases
- `ProcessResult` — exit code + combined stdout/stderr; `.success()` checks exit code

## Configuration

- Singleton: `Config::instance()` — call `initialize()` once at startup
- JSON sections: `general`, `code`, `todo`, `env`, `sync`, `projects`, `prompt`
- Typed getters/setters for every key; call `save()` to persist changes
- Tests use `Config::set_test_config_directory(path)` for isolation
- Todos stored separately at `~/.config/aliases-cli/todos.json`

## Build & Test Commands

```bash
make              # Release build → build/aliases-cli
make debug        # Debug build with -g -O0
make clean        # Remove build/
make rebuild      # clean + release
make install      # build + cp to /usr/local/bin
./run_tests.sh    # Build and run all unit tests
./build.sh -h     # Full build flag reference
```

Build uses **direct g++ invocation** (not cmake at runtime). CMakeLists.txt exists but `build.sh` is primary. Version string injected via `-DVERSION=$(git describe --tags)`.

## C++ Conventions

- **Standard**: C++17. Use `std::optional`, structured bindings, smart pointers.
- **Naming**: Classes `PascalCase`, functions/variables `snake_case`, member vars `trailing_underscore_`, constants `ALL_CAPS`
- **Error handling**: `Result<T>` for fallible operations. `std::optional<T>` for nullable returns. No exceptions in normal flow.
- **Headers**: `#pragma once`. Local: `"aliases/module.h"`. System: `<vector>`. Third-party: `"third_party/json.hpp"`.
- **Patterns**: Singleton (`Config`), PIMPL (`ProjectMapper`), static utility class (`FileUtils`, `GitOperations`, `ProcessUtils`)
- **Memory**: `shared_ptr` for shared resources, `unique_ptr` for exclusive ownership

## Adding a New Command

See `.github/instructions/new-command.instructions.md` for the full checklist. Summary:
1. Add header `include/aliases/commands/my_cmd.h` + source `src/commands/my_cmd.cpp`
2. Implement class with constructor taking `shared_ptr<ProjectMapper>` + `execute(args)` method returning `int`
3. Register in `src/main.cpp` dispatch table
4. Add any new config keys with typed getters/setters in `include/aliases/config.h` + `src/core/config.cpp`
5. Add unit tests in `tests/unit/my_cmd_test.cpp`

## Testing Conventions

See `.github/instructions/testing.instructions.md`. All tests use Google Test. Config-touching tests must use `set_test_config_directory()`. File-touching tests use `/tmp/aliases_test_$PID` temp dirs.

## Bash Integration

New CLI subcommands may need a bash wrapper in `bash_integration/` if they output shell commands that need `eval`-ing (like `project_env.sh`). Path formatting uses `ps1_wrap()` for bash readline length correctness.

## Documentation Policy

**Every code change or new feature must include a corresponding doc update.** This is non-negotiable:

- New command → add entry to `docs/reference/commands.md`
- New or changed config key → update `docs/reference/configuration.md`
- New build step or flag → update `docs/development/building.md`
- Architectural change → update `docs/development/architecture.md`
- New bash integration file → update `docs/integrations/bash-integration.md`
- **New bash integration file** → also add a `source` block for it in the `BASH_ALIASES_TEMPLATE` inside `install.sh`

If the relevant doc doesn't exist yet, create it. Never ship code without keeping the docs in sync.

## Docs

- `docs/development/architecture.md` — full architecture detail
- `docs/development/building.md` — build system detail  
- `docs/development/testing.md` — test strategy
- `docs/reference/configuration.md` — all config keys
