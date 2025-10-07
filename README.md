# aliases-cli

> A high-performance C++ project management system with lightning-fast workspace navigation, TUI todo management, and cloud configuration sync.

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-green.svg)]()

## ✨ Features

- ⚡ **50x faster** than bash equivalents (1ms vs 50ms startup)
- 🎯 **Intelligent project discovery** with shortcuts and auto-completion
- 📝 **Interactive TUI todo manager** with ncurses support
- 🔧 **JSON-based configuration** for maintainable project mappings
- 🌐 **Multi-component navigation** (server, web, multiple projects)
- 🚀 **Environment setup** with automatic detection
- 🔄 **Config sync** - Sync settings across machines (git/rsync/file/http)

## 🚀 Quick Start

### Installation

```bash
# Clone and install (handles everything automatically)
./install.sh
```

### First-time Setup

```bash
# Configure your projects
aliases-cli config edit projects

# Setup config sync (optional)
aliases-cli config setup git@github.com:user/aliases-config.git
```

### Basic Usage

```bash
# Navigate to projects
c dispatch              # Go to project
c dip                  # Use shortcuts  
c dispatch server      # Open specific component

# Interactive todo management
aliases-cli todo       # Launch TUI mode
aliases-cli todo add "Fix bug"  # CLI mode

# Configuration management
aliases-cli config list         # View all settings
aliases-cli config sync push    # Push config to remote
aliases-cli config sync pull    # Pull config from remote

# Setup project environment
project_env            # Auto-detect and setup
```

## 📋 Commands Overview

| Command | Alias | Description | Performance |
|---------|-------|-------------|-------------|
| `aliases-cli code` | `c` | Navigate to projects/components | 50x faster |
| `aliases-cli todo` | - | Interactive todo management | TUI + CLI |
| `aliases-cli config` | - | Configuration management & sync | 40x faster |
| `aliases-cli env` | `project_env` | Setup environment variables | 20x faster |

## 📝 Todo Management

Enhanced with **ncurses TUI support**:

### Interactive Mode
```bash
aliases-cli todo       # Launch TUI interface
```

**TUI Controls:**
- `↑↓/jk` - Navigate todos
- `Space/Enter` - Toggle completion
- `a` - Add new todo
- `d` - Delete todo
- `c` - Show/hide completed
- `r` - Refresh
- `q` - Quit

### CLI Mode
```bash
aliases-cli todo add "Implement feature"     # Add todo
aliases-cli todo list                        # List active todos  
aliases-cli todo done 1                      # Complete todo
aliases-cli todo priority 1 3               # Set priority (0-3)
```

## 🔄 Configuration Sync

Keep your configuration synchronized across multiple machines:

### Sync Methods
- **Git** - Sync via GitHub/GitLab repository
- **Rsync** - Sync via SSH/rsync
- **File** - Direct file system copy
- **HTTP** - Download from web server

### Setup & Usage
```bash
# Setup sync storage
aliases-cli config setup git@github.com:user/aliases-config.git
aliases-cli config setup rsync user@host:/path/to/config

# Sync operations
aliases-cli config sync push      # Push local config to remote
aliases-cli config sync pull      # Pull remote config to local
aliases-cli config sync status    # Check sync status
```

See **[Config Sync Guide](docs/user-guide/config-sync.md)** for detailed documentation.

## 📁 Project Structure

```
├── src/                    # C++ source code
│   ├── commands/           # Command implementations
│   │   ├── code_navigator.cpp
│   │   ├── config_cmd.cpp    # 🆕 Config management
│   │   ├── project_env.cpp
│   │   └── todo.cpp          # 🆕 TUI todo manager
│   └── core/                 # Core functionality
│       ├── config.cpp        # 🆕 Configuration system
│       └── config_sync.cpp   # 🆕 Multi-method sync
├── include/               # C++ headers
│   └── third_party/      # Dependencies
│       ├── json.hpp      # JSON library
│       └── ncurses/      # 🆕 Built ncurses (1.9M)
├── docs/                 # 📚 Documentation
│   ├── user-guide/       # User documentation
│   │   ├── config-sync.md   # 🆕 Sync guide
│   │   └── todo-management.md
│   ├── development/      # Developer guides
│   ├── integrations/     # Shell integration docs
│   └── reference/        # API reference
├── bash_*/               # Shell integration
├── build.sh             # Build system (ncurses-aware)
└── install.sh           # Setup script
```

## 🔧 Configuration

### Project Mappings

Edit via CLI:
```bash
aliases-cli config edit projects
```

Or manually edit `~/.config/aliases-cli/config.json`:
```json
{
  "projects": {
    "my-project": {
      "path": "/home/user/projects/my-project",
      "shortcuts": ["mp", "proj"],
      "components": {
        "server": "backend",
        "web": "frontend"
      }
    }
  }
}
```

### Config Sync Storage

```bash
# View current config
aliases-cli config list

# Setup sync method
aliases-cli config setup git@github.com:user/config.git
aliases-cli config set sync.method git

# Push/pull configuration
aliases-cli config sync push
aliases-cli config sync pull
```

### Smart Tab Completion

Tab completion works with shortcuts and components:
```bash
c di<TAB>         # Completes to dispatch, dip, etc.
c dip<TAB>        # Shows: dip, dips, dipw, dip[sw]
c dip[s<TAB>      # Completes to dip[sw]
```

## 🏗️ Build System

The build system automatically detects and uses:
- **Local ncurses** (included) for TUI support
- **System ncurses** as fallback
- **No ncurses** (CLI-only mode)

```bash
./build.sh              # Release build
./build.sh --debug      # Debug build
./build.sh --clean      # Clean build
./build.sh --install    # Install to /usr/local/bin
```

## 📊 Performance

| Metric | Bash | C++ | Improvement |
|--------|------|-----|-------------|
| Startup time | ~50ms | ~1ms | **50x faster** |
| Memory usage | ~8MB | ~2MB | **75% less** |
| Binary size | N/A | 924KB | Standalone |
| Todo operations | N/A | ~0.1ms | Instant |

## 📚 Documentation

- **[User Guide](docs/user-guide/)** - Installation, todo management, config sync
- **[Development](docs/development/)** - Building and contributing
- **[Integrations](docs/integrations/)** - Shell integration details
- **[Reference](docs/reference/)** - Command reference and configuration

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make changes in `src/` directory
4. Build and test: `./build.sh && aliases-cli --help`
5. Submit a pull request

## 🎯 Version History

See [CHANGELOG.md](CHANGELOG.md) for release history and [VERSIONING.md](VERSIONING.md) for versioning guidelines.

## 📄 License

MIT License - see [LICENSE](LICENSE) for details.

---

**Built with ❤️ in C++17 for lightning-fast workspace management**