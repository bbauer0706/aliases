#!/bin/bash
##############################################################################
#                           TEST RUNNER SCRIPT                               #
##############################################################################

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="build/tests"
VERBOSE=false
FILTER=""

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
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -f|--filter)
            FILTER="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -v, --verbose    Show verbose test output"
            echo "  -f, --filter     Run only tests matching filter (e.g., 'Common*')"
            echo "  -h, --help       Show help"
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

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    print_error "Test build directory not found: $BUILD_DIR"
    print_status "Run ./build_tests.sh first"
    exit 1
fi

# Find all test executables
TEST_EXECUTABLES=$(find "$BUILD_DIR" -type f -executable -name "*_test" 2>/dev/null | sort)

if [ -z "$TEST_EXECUTABLES" ]; then
    print_error "No test executables found in $BUILD_DIR"
    print_status "Run ./build_tests.sh to build tests"
    exit 1
fi

# Count total tests
TOTAL_TESTS=$(echo "$TEST_EXECUTABLES" | wc -l)

print_status "Found $TOTAL_TESTS test suite(s)"
echo ""

# Run each test
PASSED=0
FAILED=0
FAILED_TESTS_LIST=()

for test_bin in $TEST_EXECUTABLES; do
    test_name=$(basename "$test_bin")

    print_status "Running $test_name..."

    # Build command with optional filter
    CMD="$test_bin"
    if [ -n "$FILTER" ]; then
        CMD="$CMD --gtest_filter=$FILTER"
    fi

    # Run test
    if [ "$VERBOSE" = true ]; then
        # Show full output
        if $CMD; then
            ((PASSED++))
        else
            ((FAILED++))
            FAILED_TESTS_LIST+=("$test_name")
        fi
    else
        # Show minimal output
        TMP_OUTPUT=$(mktemp)
        if $CMD > "$TMP_OUTPUT" 2>&1; then
            ((PASSED++))
            print_success "$test_name passed"
        else
            ((FAILED++))
            FAILED_TESTS_LIST+=("$test_name")
            print_error "$test_name failed"
            cat "$TMP_OUTPUT"
        fi
        rm -f "$TMP_OUTPUT"
    fi

    echo ""
done

# Print summary
echo "========================================="
echo "Test Summary"
echo "========================================="

echo "Test suites passed: $PASSED/$TOTAL_TESTS"

if [ $FAILED -gt 0 ]; then
    print_error "Failed test suites:"
    for failed_test in "${FAILED_TESTS_LIST[@]}"; do
        echo "  - $failed_test"
    done
    echo ""
    print_error "Some tests failed!"
    exit 1
else
    echo ""
    print_success "All tests passed!"
    exit 0
fi
