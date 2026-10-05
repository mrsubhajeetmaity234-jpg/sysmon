#pragma once
#include <string>

namespace sysmon {
bool readFile(const std::string& path, std::string& out);   // read a whole (proc) file
std::string trim(const std::string& s);
std::string humanBytes(double bytes);                        // 1536 -> "1.5 KB"
std::string timestamp();                                     // "YYYY-MM-DD HH:MM:SS"
std::string bar(double percent, int width = 20);             // [#####...............]
}
