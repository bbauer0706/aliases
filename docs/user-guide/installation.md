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

# Or use force mode to skip prompts and handle conflicts
./install.sh --force
```

The installer will:
1. Build the C++ binary (or use precompiled version)
2. Set up bash integration
3. Configure shell aliases
4. Install tab completion

**Note:** If you encounter errors during installation, the script will now provide clear error messages and stop execution rather than continuing silently. Common issues include binary file being in use or permission problems.

### Force Mode

Use `./install.sh --force` for automated installations that:
- Automatically kill running aliases-cli processes
- Auto-accept all .bash_aliases updates without prompting  
- Override most conflict situations
- Provide detailed logging of all actions taken

This is useful for CI/CD pipelines, automated setups, or when you want to ensure a clean installation without manual intervention.

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

# Source bash integration
source /path/to/aliases-cli/bash_integration/project-env.sh
source /path/to/aliases-cli/bash_completion/aliases-completion.sh

# Source utility aliases (optional)
source /path/to/aliases-cli/bash_aliases/basic.ali.sh
source /path/to/aliases-cli/bash_aliases/maven.ali.sh
source /path/to/aliases-cli/bash_aliases/npm.ali.sh
```

### 3. Create Configuration

Configuration is automatically created on first run at `~/.config/aliases-cli/config.json`.

```bash
# Edit configuration
aliases-cli config edit

# Or manually edit
vim ~/.config/aliases-cli/config.json
```

## Configuration

### Project Settings

Edit your configuration using the config command or by editing `~/.config/aliases-cli/config.json`:

```json
{
  "projects": {
    "workspace_directory": "~/workspaces",
    "shortcuts": {
      "my-awesome-project": "map"
    },
    "server_paths": {
      "my-awesome-project": "backend",
      "another-project": "java"
    },
    "web_paths": {
      "my-awesome-project": "frontend",
      "another-project": "react-app"
    },
    "default_paths": {
      "server": ["java/serverJava", "serverJava", "backend", "server"],
      "web": ["webapp", "webApp", "web", "frontend", "client"]
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

### Installation Issues

**Binary in use during installation:**
```bash
# Error: "Text file busy" during install
# Option 1: Close all running aliases-cli instances manually
pkill aliases-cli
./install.sh

# Option 2: Use force mode (automatically handles this)
./install.sh --force
```

**Permission errors during installation:**
```bash
# If install fails with permission errors
# Check write permissions for:
ls -la ~/.bash_aliases ~/.bashrc ~/.local/bin/
# Fix permissions if needed:
chmod 644 ~/.bash_aliases ~/.bashrc
chmod 755 ~/.local/bin/
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
2. Verify config: `aliases-cli config list`
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