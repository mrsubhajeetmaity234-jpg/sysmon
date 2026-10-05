#pragma once
#include "process_monitor.hpp"
#include <string>
#include <vector>

namespace sysmon {
// One complete reading of the machine. Produced by the sampler thread and
// consumed by the display thread and the logger thread.
struct Snapshot {
    std::string time;

    double cpuTotal = 0;
    std::vector<double> cores;
    double load1 = 0, load5 = 0, load15 = 0;
    double ctxPerSec = 0;

    unsigned long long memTotal = 0, memUsed = 0, swapTotal = 0, swapUsed = 0;
    double memPct = 0, swapPct = 0;

    unsigned long long diskTotal = 0, diskUsed = 0;
    double diskPct = 0, diskReadBps = 0, diskWriteBps = 0;

    double netRxBps = 0, netTxBps = 0;

    size_t procCount = 0;
    std::vector<ProcInfo> top;
};
}
