#pragma once
#include "snapshot.hpp"
#include "sysinfo.hpp"
#include <string>
#include <vector>

namespace sysmon {
// Builds the terminal dashboard as a string (ANSI colours + bar graphs).
std::string render(const Snapshot& s, const SystemInfo& info,
                   const std::vector<std::string>& alerts, bool clearScreen, bool showCores);
std::string renderSystemInfo(const SystemInfo& info);
}
