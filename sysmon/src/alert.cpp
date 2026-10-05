#include "alert.hpp"
#include <cstdio>

namespace sysmon {

std::vector<std::string> checkAlerts(const Snapshot& s, const Thresholds& t) {
    std::vector<std::string> out;
    char buf[96];
    if (s.cpuTotal >= t.cpu) {
        std::snprintf(buf, sizeof buf, "CPU usage high: %.1f%% (limit %.0f%%)", s.cpuTotal, t.cpu);
        out.emplace_back(buf);
    }
    if (s.memPct >= t.mem) {
        std::snprintf(buf, sizeof buf, "Memory usage high: %.1f%% (limit %.0f%%)", s.memPct, t.mem);
        out.emplace_back(buf);
    }
    if (s.diskPct >= t.disk) {
        std::snprintf(buf, sizeof buf, "Disk usage high: %.1f%% (limit %.0f%%)", s.diskPct, t.disk);
        out.emplace_back(buf);
    }
    return out;
}

} // namespace sysmon
