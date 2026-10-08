// dinput8.dll proxy: Sekiro loads this from its own folder. We forward DirectInput to the
// system dinput8.dll and start the mod on a thread of its own (never from DllMain itself).
#include <windows.h>

#include <string>

#include "MinHook.h"
#include "clonedrone_locator.h"
#include "log.h"
#include "sheet_lookup.h"
#include "version.h"

namespace {

using DirectInput8CreateFn = HRESULT(WINAPI*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
DirectInput8CreateFn g_real_create = nullptr;
HMODULE g_real_dinput8 = nullptr;

void load_real_dinput8() {
    if (g_real_dinput8) return;
    wchar_t sys[MAX_PATH];
    GetSystemDirectoryW(sys, MAX_PATH);
    g_real_dinput8 = LoadLibraryW((std::wstring(sys) + L"\\dinput8.dll").c_str());
    if (g_real_dinput8) {
        g_real_create = reinterpret_cast<DirectInput8CreateFn>(GetProcAddress(g_real_dinput8, "DirectInput8Create"));
    }
}

std::wstring widen(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(n - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), n);
    return w;
}

std::string narrow(const std::wstring& w) {
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), n, nullptr, nullptr);
    return s;
}

std::string env(const char* name) {
    wchar_t buf[MAX_PATH * 2];
    DWORD n = GetEnvironmentVariableW(widen(name).c_str(), buf, MAX_PATH * 2);
    return (n > 0 && n < MAX_PATH * 2) ? narrow(buf) : std::string();
}

std::string steam_path_from_registry() {
    wchar_t buf[MAX_PATH];
    DWORD size = sizeof buf;
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath", RRF_RT_REG_SZ, nullptr, buf, &size) == ERROR_SUCCESS) {
        return narrow(buf);
    }
    return {};
}

std::string read_file(const std::string& path) {
    HANDLE f = CreateFileW(widen(path).c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return {};
    std::string out;
    char buf[4096];
    DWORD got = 0;
    while (ReadFile(f, buf, sizeof buf, &got, nullptr) && got > 0) out.append(buf, got);
    CloseHandle(f);
    return out;
}

std::wstring local_appdata_path(const char* relative) {
    std::wstring base = widen(env("LOCALAPPDATA"));
    std::wstring rel = widen(relative);
    for (auto& c : rel) if (c == L'/') c = L'\\';
    std::wstring full = base + L"\\" + rel;
    // create every parent folder
    for (size_t i = base.size() + 1; i < full.size(); ++i) {
        if (full[i] == L'\\') CreateDirectoryW(full.substr(0, i).c_str(), nullptr);
    }
    return full;
}

bool host_is_sekiro() {
    wchar_t exe[MAX_PATH];
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    std::wstring name(exe);
    name = name.substr(name.find_last_of(L"\\/") + 1);
    return lstrcmpiW(name.c_str(), L"sekiro.exe") == 0;
}

DWORD WINAPI mod_main(LPVOID) {
    const auto* log_row = sheets::find(sheets::paths, sheets::paths_count, "log_file");
    arena::log::open(local_appdata_path(log_row ? log_row->path : "AshinaArena/logs/latest.log"));
    arena::log::line("Ashina Arena %s loaded into sekiro.exe", ASHINA_ARENA_VERSION);

    MH_STATUS mh = MH_Initialize();
    arena::log::line("MinHook: %s", MH_StatusToString(mh));

    const auto* override_row = sheets::find(sheets::paths, sheets::paths_count, "cd_override");
    auto cd = arena::clonedrone::locate(steam_path_from_registry(),
                                        override_row ? env(override_row->path) : std::string(), read_file);
    if (cd.found) {
        arena::log::line("Clone Drone found: %s (via %s)", cd.folder.c_str(), cd.how.c_str());
    } else {
        // M1's overlay shows this in game; for now the log records it.
        arena::log::line("Clone Drone in the Danger Zone not found (%s): its commentator, music and upgrades will be missing", cd.how.c_str());
    }
    return 0;
}

}  // namespace

extern "C" __declspec(dllexport) HRESULT WINAPI DirectInput8Create(HINSTANCE inst, DWORD version, REFIID riid, LPVOID* out, LPUNKNOWN outer) {
    load_real_dinput8();
    if (!g_real_create) return E_FAIL;
    return g_real_create(inst, version, riid, out, outer);
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        if (host_is_sekiro()) {
            HANDLE t = CreateThread(nullptr, 0, mod_main, nullptr, 0, nullptr);
            if (t) CloseHandle(t);
        }
    } else if (reason == DLL_PROCESS_DETACH) {
        arena::log::close();
    }
    return TRUE;
}
