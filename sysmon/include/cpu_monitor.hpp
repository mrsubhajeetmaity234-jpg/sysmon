#pragma once
#include "monitor.hpp"
#include <chrono>
#include <vector>

namespace sysmon {

struct CpuTimes {
    unsigned long long user = 0, nice = 0, system = 0, idle = 0,
                       iowait = 0, irq = 0, softirq = 0, steal = 0;
    unsigned long long total() const { return user + nice + system + idle + iowait + irq + softirq + steal; }
    unsigned long long idleAll() const { return idle + iowait; }
};

// Reads /proc/stat and /proc/loadavg. Usage % = 100 * (1 - dIdle / dTotal) between two samples.
class CpuMonitor : public Monitor {
public:
    void sample() override;
    std::string name() const override { return "CPU"; }

    double totalUsage() const { return total_; }
    const std::vector<double>& coreUsage() const { return cores_; }
    double load1() const { return l1_; }
    double load5() const { return l5_; }
    double load15() const { return l15_; }
    double ctxSwitchesPerSec() const { return ctxRate_; }

private:
    static double usage(const CpuTimes& a, const CpuTimes& b);

    std::vector<CpuTimes> prev_, curr_;       // index 0 = all cores, 1.. = each core
    std::vector<double> cores_;
    double total_ = 0, l1_ = 0, l5_ = 0, l15_ = 0, ctxRate_ = 0;
    unsigned long long prevCtx_ = 0;
    std::chrono::steady_clock::time_point prevTime_{};
    bool haveTime_ = false;
};
}
