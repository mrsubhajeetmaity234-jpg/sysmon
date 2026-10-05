#include "disk_monitor.hpp"
#include "utils.hpp"
#include <filesystem>
#include <sstream>
#include <sys/statvfs.h>

namespace sysmon {

void DiskMonitor::sample() {
    struct statvfs sv{};
    if (statvfs(mount_.c_str(), &sv) == 0) {
        total_ = static_cast<unsigned long long>(sv.f_blocks) * sv.f_frsize;
        avail_ = static_cast<unsigned long long>(sv.f_bavail) * sv.f_frsize;
        used_ = total_ - static_cast<unsigned long long>(sv.f_bfree) * sv.f_frsize;
    }

    std::string text;
    if (!readFile("/proc/diskstats", text)) return;
    unsigned long long rd = 0, wr = 0;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream ls(line);
        unsigned long long major, minor, a, b, secRead, c, d, e, secWrite;
        std::string dev;
        if (!(ls >> major >> minor >> dev >> a >> b >> secRead >> c >> d >> e >> secWrite)) continue;
        // count only whole physical disks (they have an entry in /sys/block), skip virtual ones
        if (dev.rfind("loop", 0) == 0 || dev.rfind("ram", 0) == 0 || dev.rfind("dm-", 0) == 0 ||
            dev.rfind("zram", 0) == 0)
            continue;
        if (!std::filesystem::exists("/sys/block/" + dev)) continue;
        rd += secRead * 512ULL;
        wr += secWrite * 512ULL;
    }

    auto now = std::chrono::steady_clock::now();
    if (haveTime_) {
        double dt = std::chrono::duration<double>(now - prevTime_).count();
        if (dt > 0) {
            readBps_ = rd >= prevRead_ ? (rd - prevRead_) / dt : 0;
            writeBps_ = wr >= prevWrite_ ? (wr - prevWrite_) / dt : 0;
        }
    }
    prevRead_ = rd; prevWrite_ = wr; prevTime_ = now; haveTime_ = true;
}

} // namespace sysmon
