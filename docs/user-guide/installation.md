# Installation Guide

This guide covers installation and initial setup of aliases-cli.

## Prerequisites

- **C++17 compiler** (GCC 7+, Clang 5+)
- **Git** for repository operations
- **Bash shell** for integration features
- **Linux/macOS/WSL** (Windows support via WSL)

## Quick Installation

The simplest way to install aliases-cli:

```bash
# Clone the repository
git clone <repository-url> aliases-cli
cd aliases-cli

# Run the installer (handles everything automatically)
./install.sh
```

The installer will:
1. Build the C++ binary (or use precompiled version)
2. Set up bash integration
3. Configure shell aliases
4. Install tab completion

## Manual Installation

If you prefer manual control:

### 1. Build the Binary

```bash
# Release build
./build.sh

# Or debug build
./build.sh --debug

# Or install system-wide
./build.sh --install
```

### 2. Configure Shell Integration

Add to your `~/.bashrc` or `~/.zshrc`:

```bash
# Core aliases
alias c='aliases-cli code'
alias uw='aliases-cli update'

# Source bash integration
source /path/to/aliases-cli/bash_integration/project-env.sh
source /path/to/aliases-cli/bash_completion/aliases-completion.sh

# Source utility aliases (optional)
source /path/to/aliases-cli/bash_aliases/basic.ali.sh
source /path/to/aliases-cli/bash_aliases/maven.ali.sh
source /path/to/aliases-cli/bash_aliases/npm.ali.sh
```

### 3. Create Configuration

```bash
# Copy template
cp mappings.template.json mappings.json

# Edit with your projects
vim mappings.json
```

## Configuration

### Project Mappings

Edit `mappings.json` to configure your projects:

```json
{
  "project_mappings": {
    "my-awesome-project": {
      "shortcuts": ["map", "awesome"],
      "server_paths": ["backend", "api", "server"],
      "web_paths": ["frontend", "webapp", "ui"]
    },
    "another-project": {
      "shortcuts": ["ap"],
      "server_paths": ["java"],
      "web_paths": ["react-app"]
    }
  }
}
```

### Workspace Structure

The system expects projects in `~/workspaces/`:

```
~/workspaces/
├── project-name/
│   ├── backend/         # Server component
│   ├── frontend/        # Web component
│   └── ...
├── another-project/
│   ├── java/           # Custom server path
│   ├── react-app/      # Custom web path
│   └── ...
```

## Verification

Test your installation:

```bash
# Test basic functionality
aliases-cli --help
c --help
uw --help

# Test project navigation (if projects exist)
c <your-project>

# Test tab completion
c <TAB><TAB>

# Test todo functionality
aliases-cli todo --help
```

## Troubleshooting

### Build Issues

**Missing compiler:**
```bash
# Install GCC
sudo apt-get install build-essential  # Ubuntu/Debian
brew install gcc                      # macOS

# Or use pre-compiled binary (if available)
```

**ncurses missing:**
The build system includes ncurses, but if you encounter issues:
```bash
# This should not be necessary (ncurses is included)
sudo apt-get install libncurses5-dev  # Ubuntu/Debian
brew install ncurses                  # macOS
```

### Runtime Issues

**Command not found:**
```bash
# Ensure aliases are loaded
source ~/.bashrc

# Or use full path
/path/to/aliases-cli/build/aliases-cli --help
```

**Project not found:**
1. Check workspace structure: `~/workspaces/project-name/`
2. Verify `mappings.json` configuration
3. Test with: `aliases-cli code --list`

**Tab completion not working:**
```bash
# Ensure completion script is sourced
source bash_completion/aliases-completion.sh

# Reload shell
exec bash
```

## Advanced Configuration

### Custom Workspace Directory

By default, the system looks for projects in `~/workspaces/`. To use a different directory, modify your project mappings or use full paths.

### Multiple Workspace Roots

You can configure projects in different locations by specifying full paths in your mappings.

### System-wide Installation

```bash
# Install binary to /usr/local/bin
./build.sh --install

# Then aliases-cli will be available system-wide
```

## Updating

To update aliases-cli:

```bash
cd /path/to/aliases-cli
git pull
./build.sh
```

The installer can be run again safely to update integration scripts.

## Uninstallation

To remove aliases-cli:

1. Remove the aliases from your shell configuration
2. Remove the directory: `rm -rf /path/to/aliases-cli`
3. Remove the binary (if installed system-wide): `sudo rm /usr/local/bin/aliases-cli`