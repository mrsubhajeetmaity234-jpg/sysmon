#include "process_monitor.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <sstream>
#include <unistd.h>

namespace fs = std::filesystem;

namespace sysmon {

static bool isNumber(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c); });
}

void ProcessMonitor::sample() {
    static const long clkTck = sysconf(_SC_CLK_TCK);
    static const long pageSize = sysconf(_SC_PAGESIZE);

    auto now = std::chrono::steady_clock::now();
    double dt = haveTime_ ? std::chrono::duration<double>(now - prevTime_).count() : 0.0;

    std::vector<ProcInfo> procs;
    std::unordered_map<int, unsigned long long> ticks;

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator("/proc", ec)) {
        std::string dir = entry.path().filename().string();
        if (!isNumber(dir)) continue;

        std::string stat;
        if (!readFile("/proc/" + dir + "/stat", stat)) continue;   // process may have exited
        // Format: pid (comm) state ... ; comm may contain spaces, so use the LAST ')'
        auto lp = stat.find('('), rp = stat.rfind(')');
        if (lp == std::string::npos || rp == std::string::npos || rp < lp) continue;

        ProcInfo p;
        p.pid = std::stoi(dir);
        p.name = stat.substr(lp + 1, rp - lp - 1);

        std::istringstream rest(stat.substr(rp + 2));
        std::string tok;
        std::vector<std::string> f;
        while (rest >> tok) f.push_back(tok);
        if (f.size() < 13) continue;
        p.state = f[0][0];
        unsigned long long t = std::stoull(f[11]) + std::stoull(f[12]);   // utime + stime
        ticks[p.pid] = t;

        auto it = prevTicks_.find(p.pid);
        if (it != prevTicks_.end() && dt > 0 && t >= it->second)
            p.cpu = 100.0 * ((t - it->second) / static_cast<double>(clkTck)) / dt;

        std::string statm;
        if (readFile("/proc/" + dir + "/statm", statm)) {
            std::istringstream ms(statm);
            unsigned long long size = 0, resident = 0;
            ms >> size >> resident;
            p.rss = resident * static_cast<unsigned long long>(pageSize);
        }
        procs.push_back(std::move(p));
    }

    procs_ = std::move(procs);
    prevTicks_ = std::move(ticks);
    prevTime_ = now;
    haveTime_ = true;
}

std::vector<ProcInfo> ProcessMonitor::top(size_t n, SortKey key) const {
    std::vector<ProcInfo> v = procs_;
    auto cmp = [key](const ProcInfo& a, const ProcInfo& b) {
        return key == SortKey::Cpu ? a.cpu > b.cpu : a.rss > b.rss;
    };
    if (n > v.size()) n = v.size();
    std::partial_sort(v.begin(), v.begin() + n, v.end(), cmp);
    v.resize(n);
    return v;
}

} // namespace sysmon
