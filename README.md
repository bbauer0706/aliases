# aliases-cli

> A high-performance C++ project management system with lightning-fast workspace navigation and cloud configuration sync.

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-green.svg)]()

## ✨ Features

- ⚡ **50x faster** than bash equivalents (1ms vs 50ms startup)
- 🎯 **Intelligent project discovery** with shortcuts and auto-completion
-  **JSON-based configuration** for maintainable project mappings
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
| `aliases-cli config` | - | Configuration management & sync | 40x faster |
| `aliases-cli env` | `project_env` | Setup environment variables | 20x faster |

##  Configuration Sync

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
│   │   └── project_env.cpp
│   └── core/                 # Core functionality
│       ├── config.cpp        # 🆕 Configuration system
│       └── config_sync.cpp   # 🆕 Multi-method sync
├── include/               # C++ headers
│   └── third_party/      # Dependencies
│       └── json.hpp      # JSON library
├── docs/                 # 📚 Documentation
│   ├── user-guide/       # User documentation
│   │   └── config-sync.md   # 🆕 Sync guide
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
    "workspace_directories": ["~/workspaces", "~/dev/personal"],
    "shortcuts": {
      "my-project": "mp"
    },
    "server_paths": {
      "my-project": "backend"
    },
    "web_paths": {
      "my-project": "frontend"
    },
    "ignore": ["node_modules", "temp", ".cache"]
  }
}
```

### Multiple Workspace Sources

Scan for projects from multiple directories:

```bash
# Edit config to add multiple workspace sources
aliases-cli config edit
```

Add to the `projects` section:
```json
{
  "projects": {
    "workspace_directories": [
      "~/workspaces",
      "~/dev/personal-projects",
      "/mnt/shared/team-projects"
    ]
  }
}
```

All projects from all directories will be discovered and available via the `c` command.

### Ignoring Workspace Directories

You can configure directories to ignore when scanning your workspaces:

```bash
# Edit config to add ignore patterns
aliases-cli config edit
```

Add to the `projects` section:
```json
{
  "projects": {
    "ignore": ["build", "temp", "node_modules", ".git"]
  }
}
```

This is useful for excluding non-project directories, build artifacts, or symlinks from appearing in your project list.

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

The build system uses direct g++ invocation:

```bash
./build.sh              # Release build
./build.sh --debug      # Debug build
./build.sh --clean      # Clean build
./build.sh --install    # Install to /usr/local/bin
```

## 🧪 Testing

The project uses **Google Test** for comprehensive unit testing.

### Running Tests

```bash
# Build tests
./build_tests.sh

# Run all tests (summary output)
./run_tests.sh

# Run with verbose output
./run_tests.sh -v

# Run specific tests
./run_tests.sh -f "Common*"

# Run individual test suite
./build/tests/common_test
./build/tests/file_utils_test
```

### Test Structure

```
tests/
├── unit/              # Unit tests
│   ├── common_test.cpp
│   └── file_utils_test.cpp
└── integration/       # Integration tests (future)
```

### Writing Tests

Tests are written using Google Test framework. Example:

```cpp
#include <gtest/gtest.h>
#include "aliases/common.h"

TEST(CommonTest, TrimRemovesWhitespace) {
    EXPECT_EQ(trim("  hello  "), "hello");
}
```

See existing tests in `tests/unit/` for more examples.

## 📊 Performance

| Metric | Bash | C++ | Improvement |
|--------|------|-----|-------------|
| Startup time | ~50ms | ~1ms | **50x faster** |
| Memory usage | ~8MB | ~2MB | **75% less** |
| Binary size | N/A | 924KB | Standalone |

## 📚 Documentation

- **[User Guide](docs/user-guide/)** - Installation and config sync
- **[Development](docs/development/)** - Building and contributing
- **[Integrations](docs/integrations/)** - Shell integration details
- **[Reference](docs/reference/)** - Command reference and configuration

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make changes in `src/` directory
4. Write tests for new functionality in `tests/`
5. Build and test: `./build.sh && ./build_tests.sh && ./run_tests.sh`
6. Submit a pull request

## 🎯 Version History

See [CHANGELOG.md](CHANGELOG.md) for release history and [VERSIONING.md](VERSIONING.md) for versioning guidelines.

## 📄 License

MIT License - see [LICENSE](LICENSE) for details.

---

**Built with ❤️ in C++17 for lightning-fast workspace management**