#include "utils.hpp"
#include <cstdio>
#include <ctime>
#include <fstream>

namespace sysmon {

bool readFile(const std::string& path, std::string& out) {
    std::ifstream f(path);
    if (!f) return false;
    out.clear();
    std::string line;
    while (std::getline(f, line)) { out += line; out += '\n'; }
    return true;
}

std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

std::string humanBytes(double bytes) {
    static const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int u = 0;
    while (bytes >= 1024.0 && u < 4) { bytes /= 1024.0; ++u; }
    char buf[32];
    std::snprintf(buf, sizeof buf, "%.1f %s", bytes, units[u]);
    return buf;
}

std::string timestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}

std::string bar(double pct, int width) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    int filled = static_cast<int>(pct / 100.0 * width + 0.5);
    return "[" + std::string(filled, '#') + std::string(width - filled, '.') + "]";
}

} // namespace sysmon
