# aliases-cli Documentation

Welcome to the aliases-cli documentation. This directory contains comprehensive guides and references for using and developing aliases-cli.

## 📖 Documentation Structure

### 📘 [User Guide](user-guide/)
Complete guides for end users:
- **[Installation](user-guide/installation.md)** - Setup and configuration
- **[Todo Management](user-guide/todo-management.md)** - TUI and CLI todo system
- **[Config Sync](user-guide/config-sync.md)** - Sync configuration across machines
- **Tab Completion** - Smart completion system *(coming soon)*

### 🔧 [Development](development/)  
Documentation for developers and contributors:
- **[Building](development/building.md)** - Build system and compilation
- **Architecture** - Code organization and design *(coming soon)*
- **Contributing** - Development guidelines *(coming soon)*
- **Testing** - Test framework and coverage *(coming soon)*

### 🔗 [Integrations](integrations/)
Shell integration and compatibility:
- **[Bash Aliases](integrations/bash-aliases.md)** - Legacy and active bash utilities
- **[Bash Integration](integrations/bash-integration.md)** - Environment variable management
- **Shell Setup** - Advanced shell configuration *(coming soon)*

### 📚 [Reference](reference/)
Complete API and command references:
- **[Commands](reference/commands.md)** - All commands with options and examples
- **Configuration Schema** - JSON configuration reference *(coming soon)*
- **Exit Codes** - Complete exit code reference *(coming soon)*

## 🚀 Quick Navigation

### New Users
1. Start with **[Installation Guide](user-guide/installation.md)**
2. Read **[Command Reference](reference/commands.md)** for basic usage
3. Try **[Todo Management](user-guide/todo-management.md)** for the TUI interface

### Developers  
1. Check **[Building Guide](development/building.md)** for compilation
2. Review **[Bash Integration](integrations/bash-integration.md)** for shell features
3. Browse **[Command Reference](reference/commands.md)** for implementation details

### Power Users
1. Explore **[Todo Management](user-guide/todo-management.md)** for advanced features
2. Review **[Bash Aliases](integrations/bash-aliases.md)** for utility integration
3. Check configuration guides for customization

## 📋 Feature Overview

### Core Commands
- **`c` (code)** - Lightning-fast project navigation (50x faster than bash)
- **`todo`** - Interactive TUI todo management with ncurses
- **`config`** - Configuration management with multi-method sync (git/rsync/file/http)
- **`env`** - Automatic environment variable setup

### Key Features
- **🎯 Smart Discovery** - Auto-detect projects and components
- **⚡ Performance** - 50x faster than bash equivalents  
- **📝 TUI Interface** - Full ncurses todo management
- **🔧 JSON Configuration** - Maintainable project mappings
- **🌐 Multi-Component** - Server/web component support
- **🔄 Parallel Operations** - Efficient workspace updates

## 🏗️ Architecture Overview

### Hybrid Design
- **C++ Core** - Performance-critical operations (navigation, updates, todo)
- **Bash Integration** - Shell environment management and utilities
- **JSON Configuration** - Human-readable project mappings
- **ncurses TUI** - Rich interactive interfaces

### Dependencies
- **Built-in ncurses** - Included for TUI support (1.9M)
- **nlohmann/json** - Header-only JSON library
- **C++17 Standard** - Modern C++ features
- **Bash Integration** - For environment variable management

## 📊 Performance Metrics

| Operation | Bash Version | C++ Version | Improvement |
|-----------|-------------|-------------|-------------|
| Project navigation | ~50ms | ~1ms | **50x faster** |
| Todo operations | N/A | ~0.1ms | **Instant** |
| Config operations | ~20ms | ~0.5ms | **40x faster** |
| Memory usage | ~8MB | ~2MB | **75% less** |

## 🔍 Search and Navigation

### Finding Information
- **Commands** → [Command Reference](reference/commands.md)
- **Installation** → [Installation Guide](user-guide/installation.md)  
- **Todo System** → [Todo Management](user-guide/todo-management.md)
- **Building** → [Building Guide](development/building.md)
- **Shell Integration** → [Integrations](integrations/)

### Quick Lookup
```bash
# In the repository
find docs/ -name "*.md" -exec grep -l "keyword" {} \;
grep -r "specific topic" docs/
```

## 🆘 Getting Help

### Documentation Issues
If you find issues with documentation:
1. Check if information exists in other sections
2. Look at command help: `aliases-cli <command> --help`
3. Review source code for implementation details
4. Open an issue for missing or incorrect documentation

### Command Help
Every command includes built-in help:
```bash
aliases-cli --help              # Global help
aliases-cli code --help         # Code command help
aliases-cli todo --help         # Todo command help
aliases-cli config --help       # Config command help
```

### Quick Reference
```bash
# Most common operations
c <project>                     # Navigate to project
uw                             # Update all workspaces
aliases-cli todo               # Launch todo TUI
project_env                    # Setup environment
```

## 📝 Documentation Status

### ✅ Complete
- Installation guide
- Todo management guide  
- Config sync guide (NEW)
- Building guide
- Command reference (updated with config sync)
- Configuration reference (updated with projects section)
- Integration documentation

### 🚧 In Progress
- Advanced shell integration
- Architecture documentation (updated with config module)

### 📅 Planned
- Video tutorials
- Advanced configuration examples
- Plugin/extension system
- API documentation for embedding

---

**Last Updated:** October 7, 2025 - Added config sync documentation and updated all references to reflect current implementation

**For questions or suggestions about documentation, please open an issue in the repository.**