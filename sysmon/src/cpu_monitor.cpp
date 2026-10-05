#include "cpu_monitor.hpp"
#include "utils.hpp"
#include <sstream>

namespace sysmon {

double CpuMonitor::usage(const CpuTimes& a, const CpuTimes& b) {
    double dTotal = static_cast<double>(b.total()) - static_cast<double>(a.total());
    double dIdle = static_cast<double>(b.idleAll()) - static_cast<double>(a.idleAll());
    if (dTotal <= 0) return 0.0;
    return 100.0 * (1.0 - dIdle / dTotal);
}

void CpuMonitor::sample() {
    std::string text;
    if (!readFile("/proc/stat", text)) return;

    prev_ = curr_;
    curr_.clear();
    unsigned long long ctx = 0;

    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("cpu", 0) == 0) {
            std::istringstream ls(line);
            std::string label;
            CpuTimes t;
            ls >> label >> t.user >> t.nice >> t.system >> t.idle >> t.iowait >> t.irq >> t.softirq >> t.steal;
            curr_.push_back(t);
        } else if (line.rfind("ctxt", 0) == 0) {
            std::istringstream ls(line);
            std::string label;
            ls >> label >> ctx;
        }
    }

    auto now = std::chrono::steady_clock::now();
    if (prev_.size() == curr_.size() && !curr_.empty()) {
        total_ = usage(prev_[0], curr_[0]);
        cores_.clear();
        for (size_t i = 1; i < curr_.size(); ++i) cores_.push_back(usage(prev_[i], curr_[i]));
    }
    if (haveTime_) {
        double dt = std::chrono::duration<double>(now - prevTime_).count();
        if (dt > 0 && ctx >= prevCtx_) ctxRate_ = (ctx - prevCtx_) / dt;
    }
    prevCtx_ = ctx;
    prevTime_ = now;
    haveTime_ = true;

    std::string load;
    if (readFile("/proc/loadavg", load)) {
        std::istringstream ls(load);
        ls >> l1_ >> l5_ >> l15_;
    }
}

} // namespace sysmon
