# Architecture Overview

This document describes the architecture and design patterns used in aliases-cli.

## High-Level Architecture

aliases-cli follows a modular command-based architecture with clear separation of concerns:

```
aliases-cli
├── Core Layer (src/core/)          # Shared utilities and base functionality
├── Command Layer (src/commands/)   # Feature implementations
├── Entry Point (src/main.cpp)     # Command dispatch and CLI parsing
└── Third Party (include/third_party/) # Bundled dependencies
```

## Core Components

### Project Mapper (`project_mapper.cpp`)
- **Purpose**: Discovers and resolves project structures from configured workspace directories
- **Responsibilities**:
  - Scan workspace directories for project subdirectories
  - Resolve shorthand names (e.g. `dip` → `dispatch`) via config shortcuts
  - Detect server/web component paths per project
  - Return `ProjectInfo` structs with full path and component details
- **Pattern**: PIMPL — internal state hidden behind `std::unique_ptr<Impl>`

### String & ANSI Utilities (`common.cpp`)
- **Purpose**: Low-level string helpers and shared terminal output constants
- **Responsibilities**:
  - `trim()`, `split()`, `starts_with()`, `ends_with()`
  - `get_home_directory()`, `get_current_directory()`
  - ANSI color constants: `SUCCESS`, `ERROR`, `WARNING`, `INFO`, `RESET`, `SERVER`, `WEB`
- **Pattern**: Free functions in the `aliases` namespace

### File Utilities (`file_utils.cpp`)
- **Purpose**: Filesystem operations — path manipulation, directory traversal, file I/O
- **Responsibilities**:
  - `discover_workspace_projects()` — recursive directory listing
  - `find_component_directory()` — searches candidate paths for server/web components
  - `join_path()`, `get_basename()`, `resolve_path()`, `normalize_path()`
  - `read_file()` returns `std::optional<std::string>`; `file_exists()`, `directory_exists()`
- **Pattern**: Static utility class (no state)

### Process Utilities (`process_utils.cpp`)
- **Purpose**: subprocess execution — wraps `popen()` / `std::async`
- **Responsibilities**:
  - `execute(StringVector)` / `execute(string)` → `ProcessResult`
  - `execute_async()` → `std::future<ProcessResult>`
  - `command_exists()`, `escape_shell_argument()`, `is_port_available()`
- **Pattern**: Static utility class

### Prompt Formatter (`pwd_formatter.cpp`)
- **Purpose**: Format the current working directory for shell prompts using path-replacement rules
- **Responsibilities**:
  - Apply `PromptPathReplacement` rules from config in order (first match wins)
  - Resolve `env_var`-based or literal-`path`-based prefixes
  - Emit ANSI color codes; wrap in `\001...\002` for PS1 readline safety
- **Pattern**: Static methods (`PwdFormatter::format()`, `ansi_code()`, `ps1_wrap()`)

### Git Operations (`git_operations.cpp`)
- **Purpose**: Git repository interaction
- **Responsibilities**:
  - Repository status checking
  - Branch operations
  - Commit information extraction
- **Pattern**: Service class with command execution

### Configuration System (`config.cpp` + `config_sync.cpp`)
- **Purpose**: JSON-based configuration management with remote sync
- **Responsibilities**:
  - Load/save configuration files (config.cpp)
  - Type-safe configuration access with singleton pattern
  - Default value handling
  - Remote sync support via git/rsync/file/http (config_sync.cpp)
- **Pattern**: Singleton pattern with comprehensive getters/setters

## Command Architecture

Each command follows a consistent pattern:

```cpp
class CommandName {
public:
    explicit CommandName(std::shared_ptr<ProjectMapper> mapper);
    int execute(const StringVector& args); // returns 0=ok, 1=error, 2=usage

private:
    std::shared_ptr<ProjectMapper> mapper_;
    // command-specific state
};
```

### Todo Command (`todo.cpp`)

**Architecture Pattern:**
```
Todo Command
├── TodoManager           # Business logic and data persistence
└── Todo                 # CLI interface and argument parsing
```

### Command Registration

Commands are registered in `main.cpp` using a simple if/else dispatch:

```cpp
// Initialization
Config::instance().initialize();
auto mapper = std::make_shared<ProjectMapper>();

// Dispatch
if (command == "code" || command == "c") {
    CodeNavigator cmd(mapper);
    return cmd.execute(args);
} else if (command == "todo") {
    Todo cmd(mapper);
    return cmd.execute(args);
} // ...
```

## Data Layer

### Persistence Strategy
- **JSON files** for structured data (todos, configuration)
- **File-based** for simplicity and transparency
- **Atomic writes** to prevent corruption
- **Location**: `~/.config/aliases-cli/`

### Data Models

**TodoItem Structure:**
```cpp
struct TodoItem {
    int id;                              // Unique identifier
    std::string description;             // User-facing text
    bool completed = false;              // Completion status
    int priority = 0;                    // 0-3 priority levels
    std::string category;                // Optional categorization
    std::optional<std::time_t> due_date; // Optional due date
    std::time_t created_at;             // Creation timestamp
    std::optional<std::time_t> completed_at; // Completion timestamp
};
```

## Dependency Management

### Bundled Dependencies

**nlohmann/json (header-only):**
- Type-safe JSON parsing
- Modern C++ interface
- Error handling and validation
- No runtime dependencies

### Dependency Philosophy

1. **Minimal external dependencies** - Reduces setup complexity
2. **Bundle critical dependencies** - Ensures consistent behavior
3. **System fallback** - Use system libraries when available
4. **Graceful degradation** - Core functionality works without optional deps

## Build System Architecture

### Build Script Design (`build.sh`)

**Goals:**
- Transparent build process
- Automatic dependency detection
- Cross-platform compatibility
- Developer-friendly output

**Architecture:**
```bash
Build Process
├── Dependency Detection    # Find ncurses, check compiler
├── Flag Configuration      # Set up compile flags
├── Compilation Phase       # Build object files
└── Linking Phase          # Create final executable
```

**Key Features:**
- Parallel compilation support
- Automatic ncurses integration
- Debug/Release configurations
- Install target for system-wide access

## Error Handling Strategy

### Result Type Pattern

```cpp
template<typename T>
class Result {
    // Success/error state with typed return values
    // Prevents exception-based control flow
    // Explicit error handling at call sites
};
```

### Error Categories

1. **System errors** - File I/O, process execution
2. **User errors** - Invalid arguments, missing files
3. **Logic errors** - Programming mistakes (debug builds)
4. **Resource errors** - Memory allocation, file handles

### Recovery Strategies

- **Graceful degradation** - Disable features when dependencies missing
- **User guidance** - Clear error messages with suggested actions
- **Safe defaults** - Reasonable fallback behavior
- **State preservation** - Don't lose user data on errors

## Performance Considerations

### Startup Performance
- Lazy initialization of heavy components
- Fast argument parsing
- Minimal file I/O during startup

### TUI Performance
- Efficient screen updates (only redraw changed areas)
- Optimized data structures for large todo lists
- Responsive keyboard handling

### Memory Management
- RAII for resource management
- Smart pointers for ownership clarity
- Minimal dynamic allocation in hot paths

## Testing Strategy

### Unit Testing (Planned)
- Core utilities and business logic
- Mock interfaces for external dependencies
- Test fixtures for common scenarios

### Integration Testing
- End-to-end command execution
- File system interaction testing
- TUI behavior validation

### Manual Testing
- Cross-platform compatibility
- Terminal compatibility
- Performance with large datasets

## Future Architecture Improvements

### Plugin System
- Dynamic command loading
- Configuration-based command registration
- Third-party command extensions

### Configuration Management
- Hierarchical configuration (system, user, project)
- Schema validation
- Migration between config versions

### Data Layer Evolution
- Database backend option (SQLite)
- Sync/backup mechanisms
- Import/export capabilities

### UI Enhancements
- Multiple TUI themes
- Keyboard shortcut customization
- Mouse support

## Security Considerations

### Input Validation
- Sanitize user input in all commands
- Prevent path traversal attacks
- Validate JSON configuration

### File System Security
- Respect file permissions
- Create files with appropriate modes
- Avoid following symbolic links blindly

### Process Execution
- Validate command arguments
- Use safe process execution patterns
- Limit resource consumption

This architecture provides a solid foundation for the current feature set while allowing for future growth and enhancement.