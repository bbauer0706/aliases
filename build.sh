#!/bin/bash
##############################################################################
#                      SIMPLE G++ BUILD SCRIPT (NO CMAKE)                   #
##############################################################################

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE="Release"
CLEAN_BUILD=false
INSTALL=false
JOBS=$(nproc)
BUILD_DIR="build"
CXX="g++"
CXXFLAGS="-std=c++17 -Wall -Wextra"

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -d, --debug    Build in Debug mode"
            echo "  -c, --clean    Clean build directory"
            echo "  -i, --install  Install to /usr/local/bin"
            echo "  -j, --jobs N   Number of parallel jobs"
            echo "  -h, --help     Show help"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check for ncurses availability
NCURSES_AVAILABLE=false
NCURSES_LOCAL_PATH="include/third_party/ncurses"

if [ -d "$NCURSES_LOCAL_PATH" ]; then
    # Use our locally built ncurses
    NCURSES_AVAILABLE=true
    CXXFLAGS="$CXXFLAGS -DHAVE_NCURSES -I${NCURSES_LOCAL_PATH}/include/ncursesw"
    NCURSES_LIBS="-L${NCURSES_LOCAL_PATH}/lib -lncursesw"
    print_status "Local ncurses found at $NCURSES_LOCAL_PATH - TUI mode will be available"
elif pkg-config --exists ncurses 2>/dev/null || [ -f /usr/include/ncurses.h ] || [ -f /usr/local/include/ncurses.h ]; then
    # Use system ncurses
    NCURSES_AVAILABLE=true
    CXXFLAGS="$CXXFLAGS -DHAVE_NCURSES"
    NCURSES_LIBS="-lncurses"
    print_status "System ncurses found - TUI mode will be available"
else
    NCURSES_LIBS=""
    print_status "ncurses not found - TUI mode will be disabled, CLI mode still available"
fi

# Get version from git tag
GIT_VERSION=$(git describe --tags --abbrev=0 2>/dev/null | sed 's/^v//' || echo "dev")
CXXFLAGS="$CXXFLAGS -DVERSION=\"$GIT_VERSION\""

# Set build flags based on build type
if [[ "$BUILD_TYPE" == "Debug" ]]; then
    CXXFLAGS="$CXXFLAGS -g -O0 -DDEBUG"
else
    CXXFLAGS="$CXXFLAGS -O3 -DNDEBUG"
fi

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

print_status "Starting build process..."
print_status "Build type: $BUILD_TYPE"
print_status "Compiler: $CXX"
print_status "Flags: $CXXFLAGS"

# Clean build directory if requested
if [[ "$CLEAN_BUILD" == "true" ]]; then
    print_status "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Source files
CORE_SOURCES=(
    "src/core/project_mapper.cpp"
    "src/core/git_operations.cpp"  
    "src/core/file_utils.cpp"
    "src/core/process_utils.cpp"
    "src/core/common.cpp"
    "src/core/config.cpp"
    "src/core/config_sync.cpp"
)

COMMAND_SOURCES=(
    "src/commands/code_navigator.cpp"
    "src/commands/project_env.cpp"
    "src/commands/todo.cpp"
    "src/commands/todo_tui.cpp"
    "src/commands/config_cmd.cpp"
)

MAIN_SOURCE="src/main.cpp"

# Include directories
INCLUDES="-Iinclude"

# Build core library
print_status "Building core library..."
CORE_OBJECTS=()
for src in "${CORE_SOURCES[@]}"; do
    obj_file="$BUILD_DIR/$(basename ${src%.cpp}).o"
    print_status "Compiling $src..."
    $CXX $CXXFLAGS $INCLUDES -c "$src" -o "$obj_file"
    CORE_OBJECTS+=("$obj_file")
done

# Build command library  
print_status "Building commands library..."
COMMAND_OBJECTS=()
for src in "${COMMAND_SOURCES[@]}"; do
    obj_file="$BUILD_DIR/$(basename ${src%.cpp}).o"
    print_status "Compiling $src..."
    $CXX $CXXFLAGS $INCLUDES -c "$src" -o "$obj_file"
    COMMAND_OBJECTS+=("$obj_file")
done

# Build main executable
print_status "Building main executable..."
MAIN_OBJECT="$BUILD_DIR/main.o"
$CXX $CXXFLAGS $INCLUDES -c "$MAIN_SOURCE" -o "$MAIN_OBJECT"

# Link everything together
print_status "Linking executable..."
BINARY_PATH="$BUILD_DIR/aliases-cli"
$CXX $CXXFLAGS -o "$BINARY_PATH" "$MAIN_OBJECT" "${COMMAND_OBJECTS[@]}" "${CORE_OBJECTS[@]}" -pthread $NCURSES_LIBS

if [[ $? -ne 0 ]]; then
    print_error "Linking failed"
    exit 1
fi

print_success "Build completed successfully!"

# Test basic functionality
print_status "Testing basic functionality..."
if "$BINARY_PATH" --version &>/dev/null; then
    print_success "Basic functionality test passed"
else
    print_error "Binary created but basic test failed"
fi

# Show binary info
SIZE=$(du -h "$BINARY_PATH" | cut -f1)
print_success "Binary created: $BINARY_PATH (size: $SIZE)"

# Install if requested
if [[ "$INSTALL" == "true" ]]; then
    print_status "Installing to /usr/local/bin..."
    
    if [[ ! -w "/usr/local/bin" ]]; then
        print_status "Requesting sudo privileges for installation..."
        sudo cp "$BINARY_PATH" /usr/local/bin/
    else
        cp "$BINARY_PATH" /usr/local/bin/
    fi
    
    if [[ $? -eq 0 ]]; then
        print_success "Installation completed!"
        print_status "aliases-cli is now available system-wide"
        
        # Check if /usr/local/bin is in PATH
        if [[ ":$PATH:" != *":/usr/local/bin:"* ]]; then
            print_error "/usr/local/bin is not in your PATH"
            echo "Add this to your ~/.bashrc or ~/.zshrc:"
            echo "export PATH=\"/usr/local/bin:\$PATH\""
        fi
    else
        print_error "Installation failed"
        exit 1
    fi
fi

print_success "Build process completed successfully!"

# Show next steps
echo ""
echo "Next steps:"
echo "  - Test the binary: $BINARY_PATH --help"
if [[ "$INSTALL" != "true" ]]; then
    echo "  - Install system-wide: $0 -i"
fi
echo "  - Set up shell aliases to use the new binary"
