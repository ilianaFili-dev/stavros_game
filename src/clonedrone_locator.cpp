#include "clonedrone_locator.h"

#include <cctype>

namespace arena::clonedrone {
namespace {

// Splits VDF text into its quoted tokens and braces, unescaping "\\" inside strings.
std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> out;
    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (c == '{' || c == '}') {
            out.emplace_back(1, c);
        } else if (c == '"') {
            std::string s;
            for (++i; i < text.size() && text[i] != '"'; ++i) {
                if (text[i] == '\\' && i + 1 < text.size()) ++i;
                s += text[i];
            }
            out.push_back(s);
        }
    }
    return out;
}

std::string lower(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    char last = a.back();
    return (last == '/' || last == '\\') ? a + b : a + "/" + b;
}

}  // namespace

std::vector<std::string> parse_library_folders(const std::string& vdf_text) {
    std::vector<std::string> out;
    auto tokens = tokenize(vdf_text);
    for (size_t i = 0; i + 1 < tokens.size(); ++i) {
        if (lower(tokens[i]) == "path" && tokens[i + 1] != "{" && tokens[i + 1] != "}") {
            out.push_back(tokens[i + 1]);
        }
    }
    return out;
}

std::string parse_installdir(const std::string& acf_text) {
    auto tokens = tokenize(acf_text);
    for (size_t i = 0; i + 1 < tokens.size(); ++i) {
        if (lower(tokens[i]) == "installdir") return tokens[i + 1];
    }
    return "";
}

Location locate(const std::string& steam_path, const std::string& override_dir, ReadFile read_file) {
    if (!override_dir.empty()) {
        return {true, override_dir, "ASHINA_ARENA_CLONEDRONE_DIR"};
    }
    if (steam_path.empty()) return {false, "", "Steam not found"};

    std::vector<std::string> libraries = parse_library_folders(read_file(join(steam_path, "steamapps/libraryfolders.vdf")));
    libraries.insert(libraries.begin(), steam_path);  // the Steam folder is always a library
    for (const auto& lib : libraries) {
        std::string manifest = read_file(join(lib, std::string("steamapps/appmanifest_") + kSteamAppId + ".acf"));
        if (manifest.empty()) continue;
        std::string dir = parse_installdir(manifest);
        if (dir.empty()) continue;
        return {true, join(lib, "steamapps/common/" + dir), "Steam library " + lib};
    }
    return {false, "", "not in any Steam library"};
}

}  // namespace arena::clonedrone
