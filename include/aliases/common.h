#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <functional>

namespace aliases {

// Common types
using StringMap = std::unordered_map<std::string, std::string>;
using StringVector = std::vector<std::string>;

// Project information structure
struct ProjectInfo {
    std::string full_name;
    std::string display_name;  // Short name if available, otherwise full name
    std::string path;
    std::optional<std::string> server_path;
    std::optional<std::string> web_path;
    bool has_server_component = false;
    bool has_web_component = false;
};

// Component types
enum class ComponentType {
    MAIN,
    SERVER,
    WEB
};

// Result types for operations
template<typename T>
struct Result {
    bool success = false;
    T value = {};
    std::string error_message;
    
    static Result<T> success_with(T&& val) {
        return {true, std::forward<T>(val), ""};
    }
    
    static Result<T> error(const std::string& msg) {
        return {false, {}, msg};
    }
    
    explicit operator bool() const { return success; }
};

// Colors for terminal output
namespace Colors {
    constexpr const char* RESET = "\033[0m";
    constexpr const char* SUCCESS = "\033[1;32m";  // Bright green
    constexpr const char* ERROR = "\033[1;31m";    // Bright red
    constexpr const char* WARNING = "\033[1;33m";  // Bright yellow
    constexpr const char* INFO = "\033[1;34m";     // Bright blue
    constexpr const char* SKIPPED = "\033[1;35m";  // Bright magenta
    constexpr const char* SERVER = "\033[1;32m";   // Bright green/lime
    constexpr const char* WEB = "\033[1;34m";      // Bright blue
}

// Utility functions
std::string get_home_directory();
std::string get_workspace_directory();
std::string get_current_directory();
std::string get_script_directory();

// String utilities
std::string trim(const std::string& str);
StringVector split(const std::string& str, char delimiter);
bool starts_with(const std::string& str, const std::string& prefix);
bool ends_with(const std::string& str, const std::string& suffix);

} // namespace aliases
