#include "network_monitor.hpp"
#include "utils.hpp"
#include <sstream>

namespace sysmon {

void NetworkMonitor::sample() {
    std::string text;
    if (!readFile("/proc/net/dev", text)) return;
    std::istringstream in(text);
    std::string line;
    unsigned long long rx = 0, tx = 0;
    int lineNo = 0;
    while (std::getline(in, line)) {
        if (++lineNo <= 2) continue;               // two header lines
        auto colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string iface = trim(line.substr(0, colon));
        if (iface == "lo") continue;
        std::istringstream ls(line.substr(colon + 1));
        unsigned long long rxBytes = 0, v[7] = {0}, txBytes = 0;
        ls >> rxBytes;
        for (auto& x : v) ls >> x;                 // packets, errs, drop, fifo, frame, compressed, multicast
        ls >> txBytes;
        rx += rxBytes; tx += txBytes;
    }

    auto now = std::chrono::steady_clock::now();
    if (haveTime_) {
        double dt = std::chrono::duration<double>(now - prevTime_).count();
        if (dt > 0) {
            rxBps_ = rx >= prevRx_ ? (rx - prevRx_) / dt : 0;
            txBps_ = tx >= prevTx_ ? (tx - prevTx_) / dt : 0;
        }
    }
    prevRx_ = rx; prevTx_ = tx; prevTime_ = now; haveTime_ = true;
}

} // namespace sysmon
