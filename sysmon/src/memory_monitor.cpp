#include "memory_monitor.hpp"
#include "utils.hpp"
#include <sstream>

namespace sysmon {

void MemoryMonitor::sample() {
    std::string text;
    if (!readFile("/proc/meminfo", text)) return;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream ls(line);
        std::string key;
        unsigned long long kb = 0;
        if (!(ls >> key >> kb)) continue;
        unsigned long long bytes = kb * 1024ULL;
        if (key == "MemTotal:") total_ = bytes;
        else if (key == "MemAvailable:") available_ = bytes;
        else if (key == "SwapTotal:") swapTotal_ = bytes;
        else if (key == "SwapFree:") swapFree_ = bytes;
    }
}

} // namespace sysmon
