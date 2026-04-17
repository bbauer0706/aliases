---
description: "Testing standards for aliases-cli. Use when writing or modifying unit tests, creating test fixtures, or running tests. Covers Google Test patterns, config isolation, temp directory management, and what each test module covers."
applyTo: "tests/**"
---

# Testing Conventions for aliases-cli

## Framework & Location

- **Google Test** (gtest) from `include/third_party/googletest/`.
- All unit tests live in `tests/unit/*_test.cpp`.
- Build and run all tests: `./run_tests.sh`.
- Test binaries land in `build/tests/`.

## Test Module Responsibilities

| Test file | What it covers |
|---|---|
| `common_test.cpp` | `trim()`, `split()`, `starts_with()`, `ends_with()` |
| `config_test.cpp` | Singleton lifecycle, get/set, persistence, defaults, reset |
| `file_utils_test.cpp` | `join_path()`, `file_exists()`, `read_file()`, directory listing |
| `git_operations_test.cpp` | `get_git_status()`, `get_current_branch()`, branch checks |
| `process_utils_test.cpp` | Sync/async execution, exit codes, stdout capture |
| `project_mapper_test.cpp` | Project discovery, shorthand resolution, component paths |
| `config_sync_test.cpp` | Pull/push/status across sync methods |

## Fixture Pattern

```cpp
class MyCommandTest : public ::testing::Test {
protected:
    std::string test_dir_;

    void SetUp() override {
        test_dir_ = "/tmp/aliases_test_" + std::to_string(getpid());
        std::filesystem::create_directories(test_dir_);
        Config::set_test_config_directory(test_dir_);
        Config::instance().initialize();
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
        // reset singleton if needed
    }
};
```

## Config Isolation — REQUIRED for any config-touching test

Every test that reads or writes `Config` **must**:
1. Call `Config::set_test_config_directory(test_dir_)` before `Config::instance().initialize()`.
2. Use a per-process temp dir: `/tmp/aliases_test_<PID>` (append fixture name for parallelism).
3. Clean up in `TearDown()`.

**Never** touch `~/.config/aliases-cli/` in tests.

## Temp Directory Pattern

```cpp
// SetUp
pid_t pid = getpid();
test_dir_ = "/tmp/aliases_test_" + std::to_string(pid) + "_myfeature";
std::filesystem::create_directories(test_dir_);

// TearDown
std::filesystem::remove_all(test_dir_);
```

## Writing Good Tests

- One `TEST_F` per logical behavior, not per function.
- Name tests `VerbNoun_Condition`: `LoadsProjects_WhenFileExists`, `ReturnsError_WhenPathMissing`.
- Test both success and failure paths.
- Don't rely on test execution order — each test must be self-contained.
- Avoid `system()` for file setup; prefer `std::filesystem` or `FileUtils` helpers.

## Running Specific Tests

```bash
./build/tests/config_test                       # single binary
./build/tests/config_test --gtest_filter="*Sync*"  # filter by pattern
./run_tests.sh 2>&1 | grep FAILED             # scan for failures
```

## Adding a New Test File

1. Create `tests/unit/my_feature_test.cpp`.
2. Add the binary to `build.sh` (look for the block listing all `*_test` sources).
3. Run `./run_tests.sh` to confirm compilation and no regressions.
