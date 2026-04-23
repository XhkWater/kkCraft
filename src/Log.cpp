#include "Log.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace kk {
namespace {
std::mutex g_mutex;

std::string timestamp() {
  using namespace std::chrono;
  const auto now = system_clock::now();
  const auto t = system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  localtime_s(&tm, &t);
#else
  localtime_r(&t, &tm);
#endif
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                tm.tm_min, tm.tm_sec);
  return buf;
}

std::filesystem::path log_path() {
  // Put it next to the exe (current working directory).
  return std::filesystem::path("kkcraft.log");
}

void write_line(const char *level, std::string_view msg) {
  std::scoped_lock lock(g_mutex);
  const std::string line =
      "[" + timestamp() + "][" + level + "] " + std::string(msg) + "\n";

  // Append to file.
  std::ofstream out(log_path(), std::ios::app);
  if (out.is_open()) {
    out << line;
  }

#ifdef _WIN32
  OutputDebugStringA(line.c_str());
#endif
}
} // namespace

void log_info(std::string_view msg) { write_line("INFO", msg); }
void log_error(std::string_view msg) { write_line("ERROR", msg); }
} // namespace kk

