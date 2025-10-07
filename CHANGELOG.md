# Changelog

All notable changes to aliases-cli will be documented in this file.


## [1.1.0] - 2025-10-07

### ✨ Added
- feat: add automated CI/CD workflows (cdec280)
- feat: config (ca15479)
- feat: new stuff (9ead59a)
- feat: clear cmd not found hanlder (eed03bd)
- feat: clear (acd3924)
- feat: todo v1.1 (d106fc7)
- feat: todo v1 (40db763)
- feat: git-reset-last (cbf589c)
- feat: conditional pnpm alias (0748f7a)
- feat: tab completion for c (ef0606d)
- feat: enhance code navigator with composite project support (c519e85)
- feat: some clean up (a18d05f)
- feat: project env port selection based on user name (21988b1)

### 🐛 Fixed
- fix: update all workflow actions to v4 and add permissions (175cc6f)
- fix: update upload-artifact to v4 (3f4cd5c)
- fix: build (1b21bdf)
- fix: c completion deduplication (7468793)
- fix: manual tab completion setup (e7306e3)
- fix: is server dir compared absolute to relative path (4a68ed9)
- fix: shell wrapper to accually set the env in the shell (6525f60)
- fix: call the alias cli bin directly when sourcing (639e6ec)
- fix: stuff göll :) (70a3d9c)
- fix: project selection on shell init (9b08222)
- fix use mappings template insted of the file itself (305410a)
- fix: type in code alias (afe51cd)
- fix code script error, when dir ends on s or w (0564796)
- fix setup.sh bug (2559e5f)

### 🔧 Maintenance
- chore: build version (6461f36)
- chore: hopefully replaced all stup impls ... (3f3a037)
- chore: forgot to remove debug echo (a3630bf)


The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### In Progress
- GitHub Actions CI/CD pipeline
- Automated testing framework

## [1.0.0] - 2025-10-07

### Added
- **Config Sync System** - Multi-method configuration synchronization
  - Git-based sync with commit history
  - Rsync support for fast incremental sync
  - File copy method for local/network drives
  - HTTP method for read-only config distribution
- **Config Command** - Complete configuration management
  - `config get/set/list` - Manage settings
  - `config sync setup/pull/push/status` - Sync operations
  - `config edit/path/reset` - Configuration tools
- **Comprehensive Configuration**
  - General settings (editor, colors, verbosity)
  - Code command settings (VS Code integration)
  - Todo settings (priorities, sorting, display)
  - Environment settings (port management)
  - Sync settings (auto-sync, intervals)
  - Projects settings (workspace, shortcuts, paths)
- **Complete Documentation**
  - Config sync guide
  - Configuration reference
  - Updated command reference
  - Architecture documentation
  - Versioning guide

### Changed
- Build system now includes `config.cpp` and `config_sync.cpp`
- Configuration moved to singleton pattern
- JSON configuration stored in `~/.config/aliases-cli/config.json`
- Projects configuration integrated into main config file

### Fixed
- Config module linking issues in build system
- ProcessResult API usage in config_sync
- JSON namespace conflicts
- Hostname detection for git commits

### Technical
- Added `src/core/config.cpp` - Configuration management
- Added `src/core/config_sync.cpp` - Sync operations
- Added `src/commands/config_cmd.cpp` - Config command
- Updated `build.sh` to compile config modules
- Fixed JSON include to avoid namespace issues

## [0.9.0] - 2025-10-01

### Added
- **Todo System** - Complete task management
  - Interactive TUI mode with ncurses
  - CLI commands for scripting
  - Priority system (0-3 with visual indicators)
  - Category support
  - Search functionality
  - Completion tracking
- **Clear Command Aliases** - Comprehensive typo coverage
  - 80+ aliases for common typos
  - German keyboard (QWERTZ) support
  - Smart command-not-found handler
- **Todo Bash Aliases** - Smart shortcuts
  - `td` main alias with intelligent parameter handling
  - Priority-based creation (td-high, td-med, td-low)
  - Category shortcuts (td-bug, td-feature, etc.)
  - Search-based actions (td-done, td-rm)
  - Git integration (td-branch, td-project)

### Changed
- Improved TUI controls and navigation
- Enhanced todo sorting and filtering
- Better error messages

## [0.5.0] - 2025-09-15

### Added
- **Code Navigation** - Fast project navigation
  - Project shortcuts
  - Component detection (server/web)
  - VS Code integration
  - Tab completion
- **Environment Setup** - Project environment variables
  - Auto-detection of project context
  - Port management
  - Environment profiles
- **Project Mapper** - Intelligent project discovery
  - Auto-detect git repositories
  - Component path discovery
  - Configurable mappings

### Changed
- Migrated from bash to C++ for core operations
- 50x performance improvement for navigation
- 20x improvement for environment setup

### Deprecated
- Old bash-based navigation scripts
- Legacy project mappings in bash

## [0.1.0] - 2025-08-01

### Added
- Initial C++ implementation
- Basic project structure
- Build system with ncurses integration
- Project mapper core
- File utilities
- Git operations support

---

## Version Tags

- `v1.0.0` - Config sync & stable release (2025-10-07)
- `v0.9.0` - Todo system implementation (2025-10-01)
- `v0.5.0` - Core navigation features (2025-09-15)
- `v0.1.0` - Initial C++ implementation (2025-08-01)

## Links

- [Semantic Versioning](https://semver.org/)
- [Keep a Changelog](https://keepachangelog.com/)
- [Repository](https://github.com/bbauer0706/aliases)
