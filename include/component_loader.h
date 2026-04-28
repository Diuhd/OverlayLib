#ifndef COMPONENT_LOADER_H
#define COMPONENT_LOADER_H

#include <filesystem>
#include <string>
#include "nlohmann/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

class ComponentLoader {
    public:
        explicit ComponentLoader(fs::path component_path);
        json generate_manifest() const;

    private:
        std::string get_uri_from_path(const fs::path& path) const;
        fs::path path;
};

#endif // COMPONENT_LOADER_H
