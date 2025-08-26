#include "aliases/process_utils.h"
#include "aliases/common.h"
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <cctype>

namespace aliases {

ProcessResult ProcessUtils::execute(const std::string& command, const std::string& working_directory) {
    ProcessResult result;
    
    // Simple implementation using popen
    // In a production implementation, this would be more robust
    std::string full_command = command;
    if (!working_directory.empty()) {
        full_command = "cd " + escape_shell_argument(working_directory) + " && " + command;
    }
    
    FILE* pipe = popen((full_command + " 2>&1").c_str(), "r");
    if (!pipe) {
        result.exit_code = -1;
        result.stderr_output = "Failed to execute command";
        return result;
    }
    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result.stdout_output += buffer;
    }
    
    result.exit_code = pclose(pipe);
    return result;
}

ProcessResult ProcessUtils::execute(const StringVector& args, const std::string& working_directory) {
    std::stringstream command;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) command << " ";
        command << escape_shell_argument(args[i]);
    }
    
    return execute(command.str(), working_directory);
}

std::future<ProcessResult> ProcessUtils::execute_async(
    const std::string& command, 
    const std::string& working_directory
) {
    return std::async(std::launch::async, [command, working_directory]() {
        return execute(command, working_directory);
    });
}

bool ProcessUtils::command_exists(const std::string& command) {
    auto result = execute("which " + command + " >/dev/null 2>&1");
    return result.success();
}

std::string ProcessUtils::escape_shell_argument(const std::string& arg) {
    if (arg.empty()) return "''";
    
    bool needs_escaping = false;
    for (char c : arg) {
        if (!std::isalnum(c) && c != '/' && c != '.' && c != '_' && c != '-') {
            needs_escaping = true;
            break;
        }
    }
    
    if (!needs_escaping) return arg;
    
    std::string escaped = "'";
    for (char c : arg) {
        if (c == '\'') {
            escaped += "'\"'\"'";
        } else {
            escaped += c;
        }
    }
    escaped += "'";
    
    return escaped;
}

StringVector ProcessUtils::split_command(const std::string& command) {
    // Simple implementation - in production, this would handle quotes properly
    return split(command, ' ');
}

void ProcessUtils::wait_for_completion(std::vector<std::future<ProcessResult>>& futures) {
    for (auto& future : futures) {
        future.wait();
    }
}

bool ProcessUtils::is_port_available(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return false;
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    bool available = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0;
    close(sockfd);
    
    return available;
}

} // namespace aliases
