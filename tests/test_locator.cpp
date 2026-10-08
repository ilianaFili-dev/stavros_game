// Native (non-Windows) test of the Clone Drone locator against Steam-format files.
#include <cassert>
#include <cstdio>
#include <map>
#include <string>

#include "../src/clonedrone_locator.h"

using namespace arena::clonedrone;

static std::map<std::string, std::string> g_files;
static std::string fake_read(const std::string& p) {
    auto it = g_files.find(p);
    return it == g_files.end() ? "" : it->second;
}

static const char* kVdf = R"("libraryfolders"
{
	"0"
	{
		"path"		"C:\\Program Files (x86)\\Steam"
		"label"		""
		"apps"
		{
			"228980"		"0"
		}
	}
	"1"
	{
		"path"		"D:\\SteamLibrary"
		"apps"
		{
			"597170"		"1234"
			"814380"		"5678"
		}
	}
})";

static const char* kAcf = R"("AppState"
{
	"appid"		"597170"
	"name"		"Clone Drone in the Danger Zone"
	"installdir"		"Clone Drone in the Danger Zone"
})";

int main() {
    auto libs = parse_library_folders(kVdf);
    assert(libs.size() == 2);
    assert(libs[0] == "C:\\Program Files (x86)\\Steam");
    assert(libs[1] == "D:\\SteamLibrary");
    assert(parse_installdir(kAcf) == "Clone Drone in the Danger Zone");
    assert(parse_installdir("").empty());

    // found in a second library
    const std::string steam = "c:/program files (x86)/steam";
    g_files[steam + "/steamapps/libraryfolders.vdf"] = kVdf;
    g_files["D:\\SteamLibrary/steamapps/appmanifest_597170.acf"] = kAcf;
    auto loc = locate(steam, "", fake_read);
    assert(loc.found);
    assert(loc.folder == "D:\\SteamLibrary/steamapps/common/Clone Drone in the Danger Zone");

    // override wins
    loc = locate(steam, "E:/Games/CloneDrone", fake_read);
    assert(loc.found && loc.folder == "E:/Games/CloneDrone");

    // not installed
    g_files.clear();
    g_files[steam + "/steamapps/libraryfolders.vdf"] = kVdf;
    loc = locate(steam, "", fake_read);
    assert(!loc.found);

    // no Steam
    loc = locate("", "", fake_read);
    assert(!loc.found);

    std::puts("locator tests passed");
    return 0;
}
