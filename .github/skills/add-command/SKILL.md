---
name: add-command
description: "Add a new CLI subcommand or major feature block to aliases-cli. Use when implementing a new command end-to-end: header, implementation, main.cpp wiring, config keys, tests, build system updates, and bash integration. Handles the full pipeline including codebase exploration, design, implementation, and verification."
argument-hint: "Describe the new command: name, purpose, arguments, config it needs"
---

# Add Command Skill

Implements a complete new CLI subcommand in aliases-cli, following established codebase patterns.

## When to Use

- Adding a new top-level CLI subcommand (`aliases my-cmd ...`)
- Implementing a major feature block that spans header + source + tests + config
- Extending the project with a new core module (not just touching an existing one)

## Procedure

### Phase 1 — Understand the Request

1. Clarify (if not obvious): command name, purpose, CLI syntax, required config keys, whether it needs bash integration.
2. Study the closest existing command as a reference (e.g., `src/commands/todo.cpp` for complex state, `src/commands/config_cmd.cpp` for simple dispatch).
3. Read the dispatch table in `src/main.cpp` to understand registration.

### Phase 2 — Design

1. Define the public API: constructor signature, `execute()` args, return semantics.
2. List config keys needed (section name, key names, types, defaults).
3. Decide if a bash wrapper is needed (does output need `eval`-ing?).
4. Identify which core modules will be used (`ProjectMapper`, `FileUtils`, `GitOperations`, `ProcessUtils`).

### Phase 3 — Implement (follow `.github/instructions/new-command.instructions.md`)

Execute each step of the checklist:
1. Create `include/aliases/commands/<cmd>.h`
2. Create `src/commands/<cmd>.cpp`
3. Register in `src/main.cpp`
4. Add config keys (header → implementation → defaults → template)
5. Wire `src/commands/<cmd>.cpp` into `build.sh` SOURCES
6. Create `tests/unit/<cmd>_test.cpp` and wire into `build.sh`

### Phase 4 — Verify

```bash
make rebuild          # confirm clean build
./run_tests.sh        # confirm all tests pass
./build/aliases-cli my-cmd --help  # smoke test
```

Fix any compile errors or test failures before proceeding.

### Phase 5 — Bash Integration & Docs

- Create `bash_integration/<cmd>.sh` wrapper if the command outputs shell-evaluable code.
- Update `docs/reference/commands.md` and `docs/reference/configuration.md`.

## Key Files to Read Before Starting

- [new-command instructions](../../.github/instructions/new-command.instructions.md) — full step checklist
- [src/main.cpp](../../src/main.cpp) — dispatch table pattern
- [src/commands/todo.cpp](../../src/commands/todo.cpp) — complex command reference
- [src/commands/config_cmd.cpp](../../src/commands/config_cmd.cpp) — simple dispatch reference
- [include/aliases/config.h](../../include/aliases/config.h) — config key pattern
- [build.sh](../../build.sh) — where to add source files

## Patterns to Follow

- `Result<T>` for all fallible operations — never throw.
- `Config::instance()` for config reads — never construct Config inline.
- `ProcessUtils::execute(StringVector{...})` — prefer arg vectors over raw shell strings.
- Print to `std::cout` for normal output, `std::cerr` for errors.
- Exit code `0` = success, `1` = runtime error, `2` = bad usage/args.
