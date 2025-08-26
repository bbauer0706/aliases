#include "aliases/config_loader.h"
#include "aliases/file_utils.h"
#include "third_party/json.hpp"
#include <fstream>

using json = nlohmann::json;

namespace aliases {

bool ConfigLoader::load_local_mappings(
    StringMap& full_to_short,
    StringMap& server_paths,
    StringMap& web_paths
) const {
    auto json_file = get_mappings_file_path();
    if (!FileUtils::file_exists(json_file)) {
        return true; // Not an error if file doesn't exist
    }
    
    try {
        std::ifstream file(json_file);
        if (!file.is_open()) {
            return false;
        }
        
        json config;
        file >> config;
        
        // Load project shortcuts
        if (config.contains("project_mappings") && config["project_mappings"].contains("shortcuts")) {
            for (const auto& [key, value] : config["project_mappings"]["shortcuts"].items()) {
                full_to_short[key] = value.get<std::string>();
            }
        }
        
        // Load server paths
        if (config.contains("project_mappings") && config["project_mappings"].contains("server_paths")) {
            for (const auto& [key, value] : config["project_mappings"]["server_paths"].items()) {
                server_paths[key] = value.get<std::string>();
            }
        }
        
        // Load web paths
        if (config.contains("project_mappings") && config["project_mappings"].contains("web_paths")) {
            for (const auto& [key, value] : config["project_mappings"]["web_paths"].items()) {
                web_paths[key] = value.get<std::string>();
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        // JSON parsing failed
        return false;
    }
}

std::string ConfigLoader::get_mappings_file_path() const {
    return find_script_directory() + "/mappings.json";
}

bool ConfigLoader::has_local_mappings() const {
    return FileUtils::file_exists(get_mappings_file_path());
}

std::string ConfigLoader::find_script_directory() const {
    return get_script_directory();
}

} // namespace aliases
