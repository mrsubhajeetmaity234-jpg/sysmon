#pragma once
#include "monitor.hpp"
#include <chrono>
#include <string>

namespace sysmon {

// Space usage via statvfs(); I/O throughput via /proc/diskstats (sector = 512 bytes).
class DiskMonitor : public Monitor {
public:
    explicit DiskMonitor(std::string mountPoint = "/") : mount_(std::move(mountPoint)) {}
    void sample() override;
    std::string name() const override { return "Disk"; }

    unsigned long long total() const { return total_; }
    unsigned long long used() const { return used_; }
    double usedPercent() const { return (used_ + avail_) ? 100.0 * used_ / (used_ + avail_) : 0.0; }
    double readBytesPerSec() const { return readBps_; }
    double writeBytesPerSec() const { return writeBps_; }

private:
    std::string mount_;
    unsigned long long total_ = 0, used_ = 0, avail_ = 0;
    unsigned long long prevRead_ = 0, prevWrite_ = 0;
    double readBps_ = 0, writeBps_ = 0;
    std::chrono::steady_clock::time_point prevTime_{};
    bool haveTime_ = false;
};
}
