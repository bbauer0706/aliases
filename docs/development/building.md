# Building aliases-cli

This guide covers building aliases-cli from source, including dependencies and build options.

## Quick Build

```bash
./build.sh                    # Release build
```

The build script handles everything automatically, including ncurses integration.

## Build System

aliases-cli uses a custom bash build script (`build.sh`) instead of CMake or Makefile for simplicity and transparency.

### Build Script Options

```bash
./build.sh [OPTIONS]

Options:
  -d, --debug       Build in Debug mode (with debug symbols)
  -c, --clean       Clean build directory before building
  -i, --install     Install binary to /usr/local/bin
  -j, --jobs N      Number of parallel compilation jobs
  -h, --help        Show help message
```

### Examples

```bash
# Debug build with verbose output
./build.sh --debug

# Clean release build with 8 parallel jobs
./build.sh --clean --jobs 8

# Build and install system-wide
./build.sh --install
```

## Dependencies

### Required Dependencies

**C++ Compiler:**
- GCC 7+ or Clang 5+ (C++17 support)
- Standard library with C++17 features

**System Libraries:**
- `pthread` for threading support

### Included Dependencies

**ncurses (1.9M):**
- Pre-built and included in `include/third_party/ncurses/`
- Automatically detected and used by build system
- Enables TUI functionality

**nlohmann/json (header-only):**
- Included as `include/third_party/json.hpp`
- Used for configuration and data persistence

### Build Process

The build system performs these steps:

1. **Detect ncurses**: Checks for local build, then system installation
2. **Configure flags**: Sets up compiler flags based on available libraries
3. **Compile core**: Builds core functionality (`src/core/*.cpp`)
4. **Compile commands**: Builds command implementations (`src/commands/*.cpp`)
5. **Link executable**: Creates final binary with all dependencies

### ncurses Integration

The build system automatically handles ncurses:

```bash
# Detects local ncurses (preferred)
if [ -d "include/third_party/ncurses" ]; then
    NCURSES_AVAILABLE=true
    CXXFLAGS="$CXXFLAGS -DHAVE_NCURSES -Iinclude/third_party/ncurses/include/ncursesw"
    NCURSES_LIBS="-Linclude/third_party/ncurses/lib -lncursesw"
```

**Build modes:**
- **With local ncurses**: Full TUI support (default)
- **With system ncurses**: Falls back to system installation
- **Without ncurses**: CLI-only mode (graceful degradation)

## Build Configurations

### Release Build (Default)

```bash
./build.sh
```

**Flags:**
- `-std=c++17` - C++17 standard
- `-O3` - Aggressive optimization
- `-DNDEBUG` - Disable debug assertions
- `-Wall -Wextra` - Enable warnings

**Output:**
- Optimized binary (~924KB)
- Fast startup and execution
- No debug symbols

### Debug Build

```bash
./build.sh --debug
```

**Flags:**
- `-std=c++17` - C++17 standard
- `-g` - Debug symbols
- `-O0` - No optimization
- `-DDEBUG` - Enable debug assertions
- `-Wall -Wextra` - Enable warnings

**Output:**
- Larger binary with debug info
- Slower execution
- Debugging symbols for GDB

## Build Output

### Success Output

```
[INFO] Local ncurses found - TUI mode will be available
[INFO] Starting build process...
[INFO] Build type: Release
[INFO] Compiler: g++
[INFO] Flags: -std=c++17 -Wall -Wextra -DHAVE_NCURSES -O3 -DNDEBUG
[INFO] Building core library...
[INFO] Building commands library...
[INFO] Building main executable...
[INFO] Linking executable...
[SUCCESS] Build completed successfully!
[SUCCESS] Binary created: build/aliases-cli (size: 924K)
```

### Build Directory Structure

```
build/
├── aliases-cli              # Final executable
├── *.o                     # Object files
├── core/                   # Core object files
└── commands/               # Command object files
```

## Compilation Details

### Compiler Requirements

**GCC 7+ required for:**
- `std::optional` (C++17)
- `std::filesystem` (C++17)
- `std::string_view` (C++17)

**Clang 5+ required for:**
- Same C++17 features
- Better error messages
- Static analysis support

### Include Paths

```
-Iinclude                                              # Main headers
-Iinclude/third_party/ncurses/include/ncursesw        # ncurses headers
```

### Library Paths

```
-Linclude/third_party/ncurses/lib                     # ncurses libraries
-lncursesw                                             # Wide character ncurses
-pthread                                               # Threading support
```

### Source Organization

**Core Library (`src/core/`):**
- `project_mapper.cpp` - Project discovery and mapping
- `git_operations.cpp` - Git repository operations
- `file_utils.cpp` - File system utilities
- `config_loader.cpp` - JSON configuration loading
- `process_utils.cpp` - Process execution utilities
- `common.cpp` - Common utilities and types

**Commands (`src/commands/`):**
- `code_navigator.cpp` - Project navigation command
- `workspace_updater.cpp` - Workspace update command
- `project_env.cpp` - Environment setup command
- `todo.cpp` - Todo management CLI interface
- `todo_tui.cpp` - Todo TUI implementation with ncurses

**Main Entry Point:**
- `src/main.cpp` - Command dispatcher and argument parsing

## Platform Support

### Linux
- **Tested on**: Ubuntu 20.04+, Debian 11+
- **Compiler**: GCC 7+, Clang 5+
- **Status**: ✅ Fully supported

### macOS
- **Tested on**: macOS 11+
- **Compiler**: Clang (Xcode Command Line Tools)
- **Status**: ✅ Should work (not regularly tested)

### Windows (WSL)
- **Environment**: WSL 1/2 with Ubuntu
- **Status**: ✅ Should work via WSL

### Windows (Native)
- **Status**: ❌ Not supported (bash integration required)

## Troubleshooting

### Compilation Errors

**Missing C++17 support:**
```bash
error: 'optional' is not a member of 'std'
```
**Solution:** Upgrade to GCC 7+ or Clang 5+

**Missing headers:**
```bash
fatal error: ncurses.h: No such file or directory
```
**Solution:** This shouldn't happen (ncurses included), but check build output

### Linking Errors

**Missing pthread:**
```bash
undefined reference to 'pthread_create'
```
**Solution:** Ensure `-pthread` flag is used (automatic in build script)

**Missing ncurses:**
```bash
undefined reference to 'initscr'
```
**Solution:** Check that ncurses libraries are properly linked

### Runtime Issues

**Command not found:**
```bash
aliases-cli: command not found
```
**Solutions:**
- Use full path: `./build/aliases-cli`
- Install system-wide: `./build.sh --install`
- Add to PATH: `export PATH=$PATH:/path/to/aliases-cli/build`

### Clean Build

If you encounter persistent build issues:

```bash
# Clean everything and rebuild
rm -rf build/
./build.sh --clean
```

## Cross-Compilation

Currently not supported, but could be added by:
1. Setting appropriate compiler in build script
2. Ensuring ncurses libraries for target platform
3. Testing on target platform

## Performance Optimization

The build system is already optimized for performance:

**Compile-time optimizations:**
- `-O3` aggressive optimization
- Link-time optimization (could be added)
- Header-only JSON library (no separate compilation)

**Runtime optimizations:**
- PIMPL pattern for fast compilation
- Minimal dependencies
- Efficient algorithms and data structures

## Development Builds

For active development:

```bash
# Quick debug build with verbose output
./build.sh --debug --clean

# Development cycle
./build.sh --debug && ./build/aliases-cli --help
```

Consider using:
- IDE integration with compile_commands.json
- Static analysis tools (clang-tidy)
- Profiling tools (valgrind, perf) for performance analysis