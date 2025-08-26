#pragma once

#include "common.h"

namespace aliases {

class ConfigLoader {
public:
    ConfigLoader() = default;
    ~ConfigLoader() = default;

    // Load mappings from local configuration file
    bool load_local_mappings(
        StringMap& full_to_short,
        StringMap& server_paths,
        StringMap& web_paths
    ) const;

    // Get configuration file path
    std::string get_mappings_file_path() const;
    
    // Check if local mappings file exists
    bool has_local_mappings() const;

private:
    std::string find_script_directory() const;
};

} // namespace aliases
