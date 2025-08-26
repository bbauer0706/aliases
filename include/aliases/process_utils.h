#pragma once

#include "common.h"
#include <future>

namespace aliases {

struct ProcessResult {
    int exit_code = 0;
    std::string stdout_output;
    std::string stderr_output;
    bool success() const { return exit_code == 0; }
};

class ProcessUtils {
public:
    // Synchronous process execution
    static ProcessResult execute(const std::string& command, const std::string& working_directory = "");
    static ProcessResult execute(const StringVector& args, const std::string& working_directory = "");
    
    // Asynchronous process execution
    static std::future<ProcessResult> execute_async(
        const std::string& command, 
        const std::string& working_directory = ""
    );
    
    // Utility functions
    static bool command_exists(const std::string& command);
    static std::string escape_shell_argument(const std::string& arg);
    static StringVector split_command(const std::string& command);
    
    // Process management for parallel operations
    static void wait_for_completion(std::vector<std::future<ProcessResult>>& futures);
    static bool is_port_available(int port);
};

} // namespace aliases
