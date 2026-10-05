#pragma once
#include <string>
#include <vector>

namespace sysmon {
// Static hardware / architecture facts, collected once at start-up.
struct SystemInfo {
    std::string hostname, kernel, arch, cpuModel;
    long logicalCores = 0;
    long pageSize = 0;                    // virtual-memory page size in bytes
    unsigned long long totalRam = 0;
    bool littleEndian = true;
    std::vector<std::string> caches;      // e.g. "L1 Data 32K"

    static SystemInfo collect();
};
}
