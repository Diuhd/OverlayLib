#include "component_loader.h"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <toml++/impl/json_formatter.hpp>
#include "glib.h"
#include "glibconfig.h"
#include "nlohmann/json.hpp"
#include "toml++/toml.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

ComponentLoader::ComponentLoader(fs::path component_path): path(std::move(component_path)) {};
json ComponentLoader::generate_manifest() const {
    fs::path toml_path = path / fs::path("ovllib_config.toml");
    if (!fs::exists(toml_path)) {
        throw std::runtime_error("Invalid component: missing ovllib_config.toml");
    }
    toml::table tbl = toml::parse_file(toml_path.string());
    tbl.erase("build");

    auto proj_name = tbl["project"]["name"].value<std::string>();
    if (!proj_name.has_value()) {
        throw std::runtime_error("Invalid ovllib_config.toml: project name is invalid");
    }

    auto entry_dir = tbl["project"]["entry_dir"].value<std::string>();
    auto entry_file = tbl["project"]["entry_file"].value<std::string>();
    if (!entry_dir.has_value() || !entry_file.has_value()) {
        throw std::runtime_error("Invalid ovllib_config.toml: project.entry_dir/project.entry_file must be a string");
    }
    fs::path entry_path = path / *entry_dir / *entry_file;

    json manifest = {
        {"name", *proj_name},
        {"entry_uri", get_uri_from_path(entry_path)},
        {
            "window", {
                {"width", tbl["window"]["width"].value_or(600)},
                {"height", tbl["window"]["height"].value_or(400)},
                {"x", tbl["window"]["x"].value_or(0)},
                {"y", tbl["window"]["y"].value_or(0)},
                {"movable", tbl["window"]["movable"].value_or(true)},
                {"move_element", tbl["window"]["move_element"].value_or("")}
            }
        }
    };

    return manifest;
};


std::string ComponentLoader::get_uri_from_path(const fs::path& path) const{
    const auto host_path = fs::absolute(path);
    g_autofree char* host_uri = g_filename_to_uri(host_path.c_str(), nullptr, nullptr);
    if (host_uri == nullptr) {
        throw std::runtime_error("Invalid filename: failed to change filename to uri");
    }
    return host_uri;
}
