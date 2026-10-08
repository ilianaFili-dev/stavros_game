#include "log.h"

#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <mutex>

namespace arena::log {
namespace {
std::mutex g_mutex;
FILE* g_file = nullptr;
}  // namespace

void open(const std::wstring& path) {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_file) return;
#ifdef _WIN32
    g_file = _wfopen(path.c_str(), L"w");
#else
    g_file = std::fopen(std::string(path.begin(), path.end()).c_str(), "w");
#endif
}

void line(const char* fmt, ...) {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_file) return;
    std::time_t now = std::time(nullptr);
    char stamp[32];
    std::strftime(stamp, sizeof stamp, "%H:%M:%S", std::localtime(&now));
    std::fprintf(g_file, "[%s] ", stamp);
    va_list args;
    va_start(args, fmt);
    std::vfprintf(g_file, fmt, args);
    va_end(args);
    std::fputc('\n', g_file);
    std::fflush(g_file);
}

void close() {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_file) std::fclose(g_file);
    g_file = nullptr;
}

}  // namespace arena::log
