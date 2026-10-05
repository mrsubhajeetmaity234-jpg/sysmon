#pragma once
#include "monitor.hpp"

namespace sysmon {

// Parses /proc/meminfo (values are in kB).
class MemoryMonitor : public Monitor {
public:
    void sample() override;
    std::string name() const override { return "Memory"; }

    unsigned long long total() const { return total_; }
    unsigned long long available() const { return available_; }
    unsigned long long used() const { return total_ - available_; }
    unsigned long long swapTotal() const { return swapTotal_; }
    unsigned long long swapUsed() const { return swapTotal_ - swapFree_; }
    double usedPercent() const { return total_ ? 100.0 * used() / total_ : 0.0; }
    double swapPercent() const { return swapTotal_ ? 100.0 * swapUsed() / swapTotal_ : 0.0; }

private:
    unsigned long long total_ = 0, available_ = 0, swapTotal_ = 0, swapFree_ = 0;
};
}
