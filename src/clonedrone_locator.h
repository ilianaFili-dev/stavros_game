#pragma once
#include <string>
#include <vector>

namespace arena::clonedrone {

inline constexpr const char* kSteamAppId = "597170";

// Library folder paths listed in a Steam libraryfolders.vdf, in file order.
std::vector<std::string> parse_library_folders(const std::string& vdf_text);

// The "installdir" value of a Steam appmanifest_*.acf, or "" when absent.
std::string parse_installdir(const std::string& acf_text);

struct Location {
    bool found = false;
    std::string folder;  // the Clone Drone install folder
    std::string how;     // which route found it, for the log
};

// Finds Clone Drone: the ASHINA_ARENA_CLONEDRONE_DIR override first, then every Steam library.
// read_file returns "" for a missing file; it is injected so the logic is testable off Windows.
using ReadFile = std::string (*)(const std::string& path);
Location locate(const std::string& steam_path, const std::string& override_dir, ReadFile read_file);

}  // namespace arena::clonedrone
