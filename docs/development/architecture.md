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
- **Purpose**: Discovers and maps project structures
- **Responsibilities**: 
  - Detect project root directories
  - Identify project types (git repos, package.json, etc.)
  - Cache project metadata
- **Pattern**: Singleton-like service class

### File Utilities (`file_utils.cpp`)
- **Purpose**: Cross-platform file system operations
- **Responsibilities**:
  - Directory creation and traversal
  - File reading/writing with error handling
  - Path manipulation and validation
- **Pattern**: Static utility functions

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
    int execute(const StringVector& args);

private:
    std::shared_ptr<ProjectMapper> project_mapper_;
    // Command-specific implementation
};
```

### Todo Command (`todo.cpp` + `todo_tui.cpp`)

**Separation of Concerns:**
- `todo.cpp` - CLI interface and business logic
- `todo_tui.cpp` - Terminal UI implementation

**Architecture Pattern:**
```
Todo Command
├── TodoManager           # Business logic and data persistence
├── Todo                 # CLI interface and argument parsing
└── TodoTUI              # ncurses-based terminal interface
```

**Key Design Decisions:**
1. **Separation**: TUI code isolated in separate file for maintainability
2. **Graceful degradation**: Falls back to CLI when ncurses unavailable
3. **RAII**: Proper resource management for ncurses initialization
4. **State management**: Clean separation between UI state and data model

### Command Registration

Commands are registered in `main.cpp` using a simple dispatch pattern:

```cpp
// Command registration
commands["todo"] = [&](const StringVector& args) {
    Todo cmd(project_mapper);
    return cmd.execute(args);
};
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

## UI Architecture (TUI)

### Terminal Interface Design

**Architecture Pattern:**
```
TodoTUI
├── TuiState             # UI state management
├── Drawing subsystem    # Screen rendering
├── Input subsystem      # Keyboard handling
└── Event loop           # Main interaction loop
```

**Key Components:**

1. **State Management:**
   ```cpp
   struct TuiState {
       int current_selection;           // Currently selected item
       int scroll_offset;               // Scrolling position
       bool show_completed;             // View filter
       std::vector<TodoItem> filtered_todos; // Display list
       bool in_edit_mode;               // Input mode
       std::string edit_buffer;         // Edit text
   };
   ```

2. **Event-Driven Architecture:**
   - Main loop handles keyboard input
   - Events trigger state changes
   - State changes trigger screen redraws

3. **Graceful Degradation:**
   - Automatic fallback when ncurses unavailable
   - Stub implementations maintain API compatibility
   - Error messages guide users to CLI alternatives

### Terminal Compatibility

**TERMINFO Management:**
- Bundled terminfo database for compatibility
- Automatic environment setup
- Support for common terminal types

**Color Support:**
- Graceful fallback to monochrome
- Consistent color scheme across terminals
- Accessibility considerations

## Dependency Management

### Bundled Dependencies

**ncurses (3.0MB):**
- Self-contained installation
- Optimized terminfo database
- Wide character support (ncursesw)
- Static linking for portability

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