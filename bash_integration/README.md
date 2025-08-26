# Bash Integration

This directory contains bash shell integration scripts that provide wrapper functions around the C++ `aliases-cli` tool.

## Files

### `project-env.sh`

Provides bash wrapper functions for project environment management:

- **`project_env()`** - Sets up project environment variables by calling the C++ tool and eval'ing its output
- **`show_env()`** - Displays current environment variables using the C++ tool
- **`refresh_project_env()`** - Legacy compatibility function
- **`show_env_vars()`** - Legacy compatibility alias for `show_env()`
- **Legacy aliases** - `fix_env`, `fix_project`, `project_fix`
- **`auto_setup_new_terminal()`** - Automatically sets up environment for new terminals in workspace directories

## Why Bash Integration?

While the core `aliases-cli` tool is implemented in C++ for performance, environment variable management requires bash integration because:

1. **Environment variables** set by a child process (C++ executable) don't affect the parent shell
2. **The C++ tool outputs shell commands** that must be `eval`ed in the current shell context
3. **Shell-specific features** like auto-completion and environment management are best handled by the shell itself

## Implementation Pattern

The bash wrapper functions follow this pattern:

1. **Call the C++ tool** to generate shell commands
2. **Capture the output** (export statements)
3. **Eval the output** in the current shell to set environment variables
4. **Display success messages** from stderr

## Usage

These functions are automatically sourced by the install script and don't need to be manually loaded.

```bash
# Basic usage
project_env                    # Setup environment for current project
project_env -p 3000           # Setup with custom port
show_env                      # Display current environment variables

# Legacy compatibility
fix_env                       # Same as refresh_project_env
```

## Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌──────────────────┐
│   Bash Shell    │    │  Bash Wrapper    │    │  C++ aliases-cli │
│                 │    │                  │    │                  │
│ project_env()   │───▶│ Calls C++ tool   │───▶│ Generates export │
│                 │    │ Evals output     │◀───│ statements       │
│ Environment     │◀───│ Sets variables   │    │ Returns success  │
│ Variables Set   │    │                  │    │                  │
└─────────────────┘    └──────────────────┘    └──────────────────┘
```

This design provides:
- **Fast C++ tool** for logic and computation
- **Bash integration** for shell environment management
- **Seamless user experience** with environment variables properly set
