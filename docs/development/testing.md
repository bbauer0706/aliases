# Testing Guide

This guide covers the testing infrastructure and practices for the aliases-cli project.

## Overview

The project uses **Google Test (gtest)** v1.15.2 for unit and integration testing. The framework is vendored in `include/third_party/googletest/` for easy setup without external dependencies.

## Quick Start

```bash
# Build all tests
./build_tests.sh

# Run all tests
./run_tests.sh

# Run with verbose output
./run_tests.sh -v

# Run specific test pattern
./run_tests.sh -f "FileUtils*"
```

## Test Structure

```
tests/
├── unit/                      # Unit tests for individual components
│   ├── common_test.cpp        # String utilities, Result types
│   └── file_utils_test.cpp    # File operations, path utilities
└── integration/               # Integration tests (future)
```

## Writing Tests

### Basic Test Structure

```cpp
#include <gtest/gtest.h>
#include "aliases/your_header.h"

// Simple test
TEST(TestSuiteName, TestName) {
    EXPECT_EQ(actual_value, expected_value);
    ASSERT_TRUE(condition);
}

// Test fixture for setup/teardown
class MyTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup before each test
    }

    void TearDown() override {
        // Cleanup after each test
    }
};

TEST_F(MyTestFixture, TestName) {
    // Use fixture
}
```

### Common Assertions

```cpp
// Equality
EXPECT_EQ(a, b);      // a == b
EXPECT_NE(a, b);      // a != b

// Boolean
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Comparison
EXPECT_LT(a, b);      // a < b
EXPECT_LE(a, b);      // a <= b
EXPECT_GT(a, b);      // a > b
EXPECT_GE(a, b);      // a >= b

// String matching
EXPECT_STREQ(str1, str2);

// Fatal assertions (stop test on failure)
ASSERT_EQ(a, b);
ASSERT_TRUE(condition);
```

### Testing Optional Values

```cpp
TEST(FileUtilsTest, ReadFileReturnsOptional) {
    auto content = FileUtils::read_file("/path/to/file.txt");

    // Test successful read
    ASSERT_TRUE(content.has_value());
    EXPECT_EQ(content.value(), "expected content");

    // Test failed read
    auto missing = FileUtils::read_file("/nonexistent");
    EXPECT_FALSE(missing.has_value());
}
```

### Testing with Temporary Files

```cpp
class FileOperationsTest : public ::testing::Test {
protected:
    std::string test_dir;
    std::string test_file;

    void SetUp() override {
        // Create temp directory
        test_dir = "/tmp/test_" + std::to_string(getpid());
        system(("mkdir -p " + test_dir).c_str());

        // Create test file
        test_file = test_dir + "/file.txt";
        std::ofstream ofs(test_file);
        ofs << "test content\n";
        ofs.close();
    }

    void TearDown() override {
        // Cleanup
        system(("rm -rf " + test_dir).c_str());
    }
};

TEST_F(FileOperationsTest, CanReadFile) {
    auto content = FileUtils::read_file(test_file);
    ASSERT_TRUE(content.has_value());
    EXPECT_EQ(content.value(), "test content\n");
}
```

## Build System

### Build Script (`build_tests.sh`)

The test build script:
- Compiles Google Test library
- Builds core library for testing
- Finds and compiles all `*_test.cpp` files
- Links each test with gtest and core libraries
- Creates test executables in `build/tests/`

Options:
```bash
./build_tests.sh          # Normal build
./build_tests.sh -c       # Clean build
./build_tests.sh -j 8     # Parallel build with 8 jobs
```

### Test Runner (`run_tests.sh`)

The test runner script:
- Discovers all test executables
- Runs each test suite
- Provides summary output
- Returns non-zero exit code on failure

Options:
```bash
./run_tests.sh            # Run all tests (summary)
./run_tests.sh -v         # Verbose output
./run_tests.sh -f "Foo*"  # Filter tests
```

## Adding New Tests

1. **Create test file**: `tests/unit/my_feature_test.cpp`

```cpp
#include <gtest/gtest.h>
#include "aliases/my_feature.h"

TEST(MyFeatureTest, BasicFunctionality) {
    auto result = my_function();
    EXPECT_TRUE(result.success);
}
```

2. **Build tests**:
```bash
./build_tests.sh
```

3. **Run tests**:
```bash
./run_tests.sh
```

The build system automatically discovers new test files matching `*_test.cpp`.

## Best Practices

### Do's

✅ **Write tests for new features** before implementing them (TDD)

✅ **Test edge cases**: empty strings, null values, boundary conditions

✅ **Use descriptive test names**: `TEST(FileUtils, ReturnsEmptyForNonexistentFile)`

✅ **Keep tests isolated**: Each test should be independent

✅ **Use fixtures** for common setup/teardown

✅ **Test both success and failure paths**

### Don'ts

❌ **Don't use global state** between tests

❌ **Don't depend on test execution order**

❌ **Don't test implementation details**, test behavior

❌ **Don't write overly complex tests** - keep them simple and focused

❌ **Don't skip cleanup** in TearDown()

## Test Coverage

Currently tested modules:
- ✅ String utilities (`common.cpp`)
- ✅ Result types (`common.h`)
- ✅ File utilities (`file_utils.cpp`)
- ✅ Path operations (`file_utils.cpp`)

Planned for testing:
- ⏳ Project mapper
- ⏳ Git operations
- ⏳ Configuration management
- ⏳ Config sync
- ⏳ Todo management

## CI/CD Integration

Tests can be integrated into CI/CD pipelines:

```bash
# In your CI script
./build.sh || exit 1           # Build main binary
./build_tests.sh || exit 1     # Build tests
./run_tests.sh || exit 1       # Run tests (fails on error)
```

## Debugging Tests

### Run specific test
```bash
./build/tests/common_test --gtest_filter="CommonTest.TrimRemovesWhitespace"
```

### List available tests
```bash
./build/tests/common_test --gtest_list_tests
```

### Run with verbose output
```bash
./build/tests/common_test --gtest_verbose
```

### Repeat tests
```bash
./build/tests/common_test --gtest_repeat=10
```

## Troubleshooting

### Tests won't compile

1. Check Google Test is present:
```bash
ls -la include/third_party/googletest/
```

2. Clean and rebuild:
```bash
./build_tests.sh -c
```

### Test failures

1. Run with verbose output:
```bash
./run_tests.sh -v
```

2. Run specific failing test:
```bash
./build/tests/common_test --gtest_filter="*FailingTest*"
```

3. Check test expectations match actual implementation

## Resources

- [Google Test Documentation](https://google.github.io/googletest/)
- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Advanced Testing Guide](https://google.github.io/googletest/advanced.html)

## Contributing Tests

When submitting a PR:

1. ✅ Add tests for new functionality
2. ✅ Ensure all existing tests pass
3. ✅ Include both positive and negative test cases
4. ✅ Update this guide if adding new testing patterns

```bash
# Before submitting PR
./build.sh && ./build_tests.sh && ./run_tests.sh
```
