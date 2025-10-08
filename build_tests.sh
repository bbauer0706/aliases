#!/bin/bash
##############################################################################
#                           TEST BUILD SCRIPT                                #
##############################################################################

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE="Debug"  # Tests should always build with debug symbols
CLEAN_BUILD=false
JOBS=$(nproc)
BUILD_DIR="build/tests"
CXX="g++"
CXXFLAGS="-std=c++17 -Wall -Wextra -g -O0"

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
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -c, --clean    Clean build directory"
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

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

print_status "Starting test build process..."
print_status "Build type: $BUILD_TYPE"
print_status "Compiler: $CXX"

# Clean build directory if requested
if [[ "$CLEAN_BUILD" == "true" ]]; then
    print_status "Cleaning test build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Google Test paths
GTEST_DIR="include/third_party/googletest/googletest"
GTEST_INCLUDE="-Iinclude/third_party/googletest/googletest/include"
GTEST_SRC="$GTEST_DIR/src/gtest-all.cc"
GTEST_MAIN="$GTEST_DIR/src/gtest_main.cc"

# Check if Google Test exists
if [ ! -d "$GTEST_DIR" ]; then
    print_error "Google Test not found at $GTEST_DIR"
    print_error "Please run: git submodule update --init --recursive"
    print_error "Or download Google Test manually to $GTEST_DIR"
    exit 1
fi

# Build Google Test library
print_status "Building Google Test library..."
GTEST_ALL_OBJ="$BUILD_DIR/gtest-all.o"
GTEST_MAIN_OBJ="$BUILD_DIR/gtest_main.o"

if [ ! -f "$GTEST_ALL_OBJ" ] || [[ "$CLEAN_BUILD" == "true" ]]; then
    print_status "Compiling gtest-all.cc..."
    $CXX $CXXFLAGS -I"$GTEST_DIR" $GTEST_INCLUDE -c "$GTEST_SRC" -o "$GTEST_ALL_OBJ" -pthread
fi

if [ ! -f "$GTEST_MAIN_OBJ" ] || [[ "$CLEAN_BUILD" == "true" ]]; then
    print_status "Compiling gtest_main.cc..."
    $CXX $CXXFLAGS -I"$GTEST_DIR" $GTEST_INCLUDE -c "$GTEST_MAIN" -o "$GTEST_MAIN_OBJ" -pthread
fi

# Include directories
INCLUDES="-Iinclude $GTEST_INCLUDE"

# Core source files (needed for testing)
CORE_SOURCES=(
    "src/core/project_mapper.cpp"
    "src/core/git_operations.cpp"
    "src/core/file_utils.cpp"
    "src/core/process_utils.cpp"
    "src/core/common.cpp"
    "src/core/config.cpp"
    "src/core/config_sync.cpp"
    "src/commands/todo.cpp"
    "src/commands/todo_tui.cpp"
)

# Build core library for tests
print_status "Building core library for tests..."
CORE_OBJECTS=()
for src in "${CORE_SOURCES[@]}"; do
    obj_file="$BUILD_DIR/$(basename ${src%.cpp}).o"
    if [ ! -f "$obj_file" ] || [[ "$CLEAN_BUILD" == "true" ]] || [ "$src" -nt "$obj_file" ]; then
        print_status "Compiling $src..."
        $CXX $CXXFLAGS $INCLUDES -c "$src" -o "$obj_file"
    fi
    CORE_OBJECTS+=("$obj_file")
done

# Find all test files
print_status "Finding test files..."
TEST_FILES=$(find tests -name "*_test.cpp" 2>/dev/null || echo "")

if [ -z "$TEST_FILES" ]; then
    print_status "No test files found (*_test.cpp)"
    print_status "Create test files in tests/unit/ or tests/integration/"
    exit 0
fi

# Build and link each test
print_status "Building test executables..."
TEST_EXECUTABLES=()

for test_src in $TEST_FILES; do
    test_name=$(basename ${test_src%.cpp})
    test_obj="$BUILD_DIR/${test_name}.o"
    test_bin="$BUILD_DIR/${test_name}"

    print_status "Compiling $test_src..."
    $CXX $CXXFLAGS $INCLUDES -c "$test_src" -o "$test_obj"

    print_status "Linking $test_name..."
    $CXX $CXXFLAGS -o "$test_bin" "$test_obj" "${CORE_OBJECTS[@]}" \
        "$GTEST_ALL_OBJ" "$GTEST_MAIN_OBJ" -pthread \
        -Linclude/third_party/ncurses/lib -lncursesw

    TEST_EXECUTABLES+=("$test_bin")
done

print_success "Test build completed successfully!"
print_status "Built ${#TEST_EXECUTABLES[@]} test executable(s):"
for test_bin in "${TEST_EXECUTABLES[@]}"; do
    echo "  - $test_bin"
done

echo ""
echo "Next steps:"
echo "  - Run all tests: ./run_tests.sh"
echo "  - Run specific test: $BUILD_DIR/<test_name>"
