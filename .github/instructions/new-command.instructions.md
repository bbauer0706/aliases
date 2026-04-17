---
description: "Use when adding a new CLI subcommand, implementing a new feature block, or extending an existing command in aliases-cli. Covers the full checklist: header, implementation, main.cpp registration, config keys, tests, bash integration, and build system wiring."
---

# Adding a New Command to aliases-cli

Follow every step in order. Do not skip steps — the build will fail silently if source files aren't added to `build.sh`.

## 1. Header — `include/aliases/commands/my_cmd.h`

```cpp
#pragma once
#include "aliases/project_mapper.h"
#include "aliases/common.h"
#include <memory>
#include <string>

namespace aliases {

class MyCommand {
public:
    explicit MyCommand(std::shared_ptr<ProjectMapper> mapper);
    int execute(const StringVector& args);

private:
    std::shared_ptr<ProjectMapper> mapper_;
    // add private helpers here
};

} // namespace aliases
```

Rules:
- Constructor takes `shared_ptr<ProjectMapper>` (only; don't reach into `Config` at construction time).
- `execute()` returns `int` exit code — `0` success, `1` error, `2` usage/bad args.
- One class per file.

## 2. Implementation — `src/commands/my_cmd.cpp`

```cpp
#include "aliases/commands/my_cmd.h"
#include "aliases/config.h"
#include "aliases/common.h"
#include <iostream>

namespace aliases {

MyCommand::MyCommand(std::shared_ptr<ProjectMapper> mapper)
    : mapper_(std::move(mapper)) {}

int MyCommand::execute(const StringVector& args) {
    if (args.empty()) {
        std::cerr << "Usage: aliases my-cmd <subcommand>\n";
        return 2;
    }
    // ...
    return 0;
}

} // namespace aliases
```

Print errors to `std::cerr`. Print output to `std::cout`. Use `Config::instance()` to read settings.

## 3. Register in `src/main.cpp`

Find the command dispatch block and add:

```cpp
#include "aliases/commands/my_cmd.h"

// In the dispatch table:
} else if (command == "my-cmd") {
    MyCommand cmd(mapper);
    return cmd.execute(args);
```

Look for the existing pattern (`} else if (command == "code") {`) to place it in alphabetical order.

## 4. Add Config Keys (if needed) — `include/aliases/config.h` + `src/core/config.cpp`

**Header** — add typed getters/setters in the appropriate section (create a new section if needed):
```cpp
// my_cmd section
std::string get_my_cmd_option() const;
void set_my_cmd_option(const std::string& value);
```

**Implementation** — follow the existing pattern: read from `config_data_["my_cmd"]["option"]` with a default fallback:
```cpp
std::string Config::get_my_cmd_option() const {
    return config_data_.value("/my_cmd/option"_json_pointer, std::string("default_value"));
}
void Config::set_my_cmd_option(const std::string& value) {
    config_data_["my_cmd"]["option"] = value;
}
```

Also add the key to `reset_to_defaults()` and `config.template.json`.

## 5. Wire into `build.sh`

Find the `SOURCES` array (or equivalent) and add:
- `src/commands/my_cmd.cpp`

Find the test binary block and add the new test binary target there too (see step 6).

## 6. Unit Tests — `tests/unit/my_cmd_test.cpp`

```cpp
#include <gtest/gtest.h>
#include "aliases/commands/my_cmd.h"
#include "aliases/project_mapper.h"
#include "aliases/config.h"
#include <filesystem>

class MyCommandTest : public ::testing::Test {
protected:
    std::string test_dir_;
    std::shared_ptr<aliases::ProjectMapper> mapper_;

    void SetUp() override {
        test_dir_ = "/tmp/aliases_test_" + std::to_string(getpid()) + "_mycmd";
        std::filesystem::create_directories(test_dir_);
        aliases::Config::set_test_config_directory(test_dir_);
        aliases::Config::instance().initialize();
        mapper_ = std::make_shared<aliases::ProjectMapper>();
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir_);
    }
};

TEST_F(MyCommandTest, ReturnsErrorOnEmptyArgs) {
    aliases::MyCommand cmd(mapper_);
    EXPECT_EQ(cmd.execute({}), 2);
}
```

See `.github/instructions/testing.instructions.md` for full testing conventions.

## 7. Bash Integration (if the command outputs shell code)

If your command outputs lines meant to be `eval`-ed (env vars, `cd` commands, etc.):

1. Create `bash_integration/my-cmd.sh`:
```bash
my_cmd() {
    local result
    result=$(aliases-cli my-cmd "$@") || return $?
    eval "$result"
}
```
2. Add a `source` block for it inside the `BASH_ALIASES_TEMPLATE` in `install.sh`:
```bash
# Load bash integration for my-cmd
if [ -f "\$ALIASES_DIR/bash_integration/my-cmd.sh" ]; then
  source "\$ALIASES_DIR/bash_integration/my-cmd.sh"
fi
```

If the command only prints data (like `todo list`), no wrapper needed and no install.sh change needed.

## 8. Documentation

Update:
- `docs/reference/commands.md` — add entry for the new subcommand
- `docs/reference/configuration.md` — list any new config keys
- `README.md` — add to the features/commands section if user-facing

## Checklist

- [ ] `include/aliases/commands/my_cmd.h` created
- [ ] `src/commands/my_cmd.cpp` created
- [ ] Registered in `src/main.cpp` dispatch table
- [ ] Config keys added to header + implementation + defaults + template (if any)
- [ ] `src/commands/my_cmd.cpp` added to `build.sh` sources
- [ ] `tests/unit/my_cmd_test.cpp` created and added to `build.sh`
- [ ] `./run_tests.sh` passes
- [ ] Bash wrapper created in `bash_integration/` (if eval needed)
- [ ] `install.sh` `BASH_ALIASES_TEMPLATE` updated (if bash wrapper created)
- [ ] Docs updated
