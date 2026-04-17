---
description: "C++ coding standards for aliases-cli. Use when writing or modifying .cpp or .h source files. Covers naming, error handling, includes, memory, ANSI output, and patterns used in this codebase."
applyTo: ["src/**/*.cpp", "src/**/*.h", "include/**/*.h"]
---

# C++ Conventions for aliases-cli

## Standard & Compiler

- C++17. Build via `build.sh` (g++ invocation) or `make`. No MSVC support.
- Compiler flags: `-std=c++17 -O3` (release) / `-g -O0` (debug).

## Naming

| Entity | Style | Example |
|---|---|---|
| Class / Struct / Enum | PascalCase | `ProjectMapper`, `ComponentType` |
| Public/private function | snake_case | `get_project_info()`, `execute_command()` |
| Member variable | trailing underscore | `mapper_`, `config_data_` |
| Local variable & param | snake_case | `project_name`, `base_port` |
| Constant / macro | ALL_CAPS | `PROGRAM_NAME`, `VERSION` |
| Type alias | PascalCase or snake_case | `StringMap`, `StringVector` |

## Header Guards & Includes

```cpp
#pragma once            // always, never #ifndef guards

#include "aliases/common.h"        // local project headers — quotes, relative to include/
#include "third_party/json.hpp"    // third-party — quotes
#include <vector>                   // system/STL — angle brackets
```

Order: local project headers → third-party → system headers, each group separated by blank line.

## Error Handling

- Use `Result<T>` (defined in `include/aliases/common.h`) for any operation that can fail:
  ```cpp
  Result<string> open_project(const string& name) {
      if (!found) return Result<string>::error("Project not found: " + name);
      return Result<string>::success_with(path);
  }
  ```
- Return `std::optional<T>` for "might not exist" without an error message.
- **No exceptions** in normal control flow. Exceptions only for fatal/unrecoverable conditions.
- Check `ProcessResult.success()` (exit code == 0) after every `ProcessUtils::execute()` call.

## Memory Ownership

- `std::shared_ptr<T>` — shared ownership (e.g., `shared_ptr<ProjectMapper>` passed to commands).
- `std::unique_ptr<T>` — exclusive ownership (e.g., PIMPL `impl_`).
- Avoid raw `new`/`delete`. Use `std::make_shared` / `std::make_unique`.

## Class Patterns

### Singleton (`Config`)
```cpp
static Config& instance();   // static getter
// private constructor, deleted copy/assign
```
Call `Config::instance().initialize()` **once** at startup (`src/main.cpp`).

### PIMPL (`ProjectMapper`)
```cpp
class ProjectMapper {
    class Impl;
    std::unique_ptr<Impl> impl_;
public:
    explicit ProjectMapper(std::shared_ptr<Config>);
};
```
Use when internal state is complex or compile-time isolation is valuable.

### Static Utility Class (`FileUtils`, `GitOperations`, `ProcessUtils`)
```cpp
class FileUtils {
public:
    static bool file_exists(const std::string& path);
    // no instance; no state
};
```

### Command Class (`CodeNavigator`, etc.)
```cpp
class MyCommand {
public:
    explicit MyCommand(std::shared_ptr<ProjectMapper> mapper);
    int execute(const StringVector& args);
private:
    std::shared_ptr<ProjectMapper> mapper_;
};
```
- Constructor takes `shared_ptr<ProjectMapper>` (and nothing else unless absolutely needed).
- `execute()` returns exit code (`0` = success, non-zero = error).

## ANSI Terminal Output

- Use constants from `aliases/common.h`: `SUCCESS`, `ERROR`, `WARNING`, `INFO`, `SKIPPED`, `SERVER`, `WEB`, `RESET`.
- Always check `Config::instance().get_terminal_colors()` before emitting color codes.
- For shell prompt wrapping, use `PwdFormatter::ps1_wrap()` to avoid readline length issues.

## JSON (nlohmann/json)

Use `#include "third_party/json.hpp"` and `using json = nlohmann::json;`.
Always wrap JSON parsing in try/catch — only place where exceptions are acceptable:
```cpp
try {
    auto j = json::parse(content);
} catch (const json::exception& e) {
    return Result<Config>::error(string("JSON parse error: ") + e.what());
}
```

## ProcessUtils

- Prefer `ProcessUtils::execute(StringVector{...})` (arg vector) over raw string commands to avoid shell injection.
- Use `escape_shell_argument()` if you must build a shell string.
- Async operations return `std::future<ProcessResult>` via `execute_async()`.

## File Layout

```
include/aliases/commands/my_cmd.h    ← declaration
src/commands/my_cmd.cpp              ← implementation
```
One primary class per file. Free utility functions follow the same grouping principle.
