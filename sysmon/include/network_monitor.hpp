#pragma once
#include "monitor.hpp"
#include <chrono>

namespace sysmon {

// Sums rx/tx bytes of all non-loopback interfaces from /proc/net/dev and converts to B/s.
class NetworkMonitor : public Monitor {
public:
    void sample() override;
    std::string name() const override { return "Network"; }
    double rxBytesPerSec() const { return rxBps_; }
    double txBytesPerSec() const { return txBps_; }

private:
    unsigned long long prevRx_ = 0, prevTx_ = 0;
    double rxBps_ = 0, txBps_ = 0;
    std::chrono::steady_clock::time_point prevTime_{};
    bool haveTime_ = false;
};
}
