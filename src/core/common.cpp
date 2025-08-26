#include "aliases/common.h"
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace aliases {

std::string get_home_directory() {
    const char* home = std::getenv("HOME");
    return home ? std::string(home) : "/tmp";
}

std::string get_workspace_directory() {
    return get_home_directory() + "/workspaces";
}

std::string get_current_directory() {
    char* cwd = getcwd(nullptr, 0);
    if (!cwd) return "";
    std::string result(cwd);
    free(cwd);
    return result;
}

std::string get_script_directory() {
    // For now, assume it's the same as workspace/aliases
    return get_workspace_directory() + "/aliases";
}

std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

StringVector split(const std::string& str, char delimiter) {
    StringVector result;
    std::istringstream stream(str);
    std::string token;
    
    while (std::getline(stream, token, delimiter)) {
        result.push_back(token);
    }
    
    return result;
}

bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           str.compare(0, prefix.size(), prefix) == 0;
}

bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

} // namespace aliases
