cmake_minimum_required(VERSION 3.16)
project(aliases-cli VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wall -Wextra")
endif()

# Include directories
include_directories(include)

# Find required packages
find_package(Threads REQUIRED)
find_package(Curses REQUIRED)

# Core library
set(CORE_SOURCES
    src/core/project_mapper.cpp
    src/core/git_operations.cpp
    src/core/file_utils.cpp
    src/core/config.cpp
    src/core/config_sync.cpp
    src/core/process_utils.cpp
)

set(CORE_HEADERS
    include/aliases/project_mapper.h
    include/aliases/git_operations.h
    include/aliases/file_utils.h
    include/aliases/config.h
    include/aliases/config_sync.h
    include/aliases/process_utils.h
    include/aliases/common.h
)

# Create core library
add_library(aliases_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_link_libraries(aliases_core Threads::Threads)

# Command implementations
set(COMMAND_SOURCES
    src/commands/code_navigator.cpp
    src/commands/project_env.cpp
    src/commands/todo.cpp
    src/commands/todo_tui.cpp
    src/commands/config_cmd.cpp
)

set(COMMAND_HEADERS
    include/aliases/commands/code_navigator.h
    include/aliases/commands/project_env.h
    include/aliases/commands/todo.h
    include/aliases/commands/todo_tui.h
    include/aliases/commands/config_cmd.h
)

# Create commands library
add_library(aliases_commands STATIC ${COMMAND_SOURCES} ${COMMAND_HEADERS})
target_link_libraries(aliases_commands aliases_core ${CURSES_LIBRARIES})
target_include_directories(aliases_commands PRIVATE ${CURSES_INCLUDE_DIR})

# Main executable
add_executable(aliases-cli src/main.cpp)
target_link_libraries(aliases-cli aliases_commands aliases_core Threads::Threads)

# Install targets
install(TARGETS aliases-cli DESTINATION bin)

# Option to build tests
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
