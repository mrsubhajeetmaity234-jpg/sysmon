#pragma once
#include "monitor.hpp"
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace sysmon {

struct ProcInfo {
    int pid = 0;
    std::string name;
    char state = '?';
    double cpu = 0.0;                  // % of one core
    unsigned long long rss = 0;        // resident memory in bytes
};

enum class SortKey { Cpu, Memory };

// Walks /proc/<pid>/stat and /proc/<pid>/statm.
class ProcessMonitor : public Monitor {
public:
    void sample() override;
    std::string name() const override { return "Processes"; }

    std::vector<ProcInfo> top(size_t n, SortKey key) const;
    size_t count() const { return procs_.size(); }

private:
    std::vector<ProcInfo> procs_;
    std::unordered_map<int, unsigned long long> prevTicks_;   // pid -> utime+stime
    std::chrono::steady_clock::time_point prevTime_{};
    bool haveTime_ = false;
};
}
