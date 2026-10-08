#pragma once
#include <string>

namespace arena::log {

// Opens (truncates) the log file. Safe to call once; later calls are ignored.
void open(const std::wstring& path);
void line(const char* fmt, ...);
void close();

}  // namespace arena::log
