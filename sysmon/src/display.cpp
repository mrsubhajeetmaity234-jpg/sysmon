#include "display.hpp"
#include "utils.hpp"
#include <cstdio>
#include <sstream>

namespace sysmon {

static const char* RESET = "\033[0m";
static const char* BOLD = "\033[1m";
static const char* RED = "\033[31m";
static const char* YEL = "\033[33m";
static const char* GRN = "\033[32m";
static const char* CYN = "\033[36m";

static const char* colourFor(double pct) { return pct >= 85 ? RED : (pct >= 60 ? YEL : GRN); }

static std::string pctBar(double pct) {
    char num[16];
    std::snprintf(num, sizeof num, "%5.1f%%", pct);
    return std::string(colourFor(pct)) + bar(pct) + " " + num + RESET;
}

std::string renderSystemInfo(const SystemInfo& i) {
    std::ostringstream o;
    o << BOLD << CYN << "== System / Architecture ==" << RESET << "\n"
      << "Host    : " << i.hostname << "  (" << i.kernel << ")\n"
      << "CPU     : " << i.cpuModel << "\n"
      << "Arch    : " << i.arch << ", " << i.logicalCores << " logical cores, "
      << (i.littleEndian ? "little" : "big") << "-endian\n"
      << "RAM     : " << humanBytes(static_cast<double>(i.totalRam)) << ", page size " << i.pageSize << " B\n"
      << "Caches  : ";
    if (i.caches.empty()) o << "n/a";
    for (size_t k = 0; k < i.caches.size(); ++k) o << (k ? " | " : "") << i.caches[k];
    o << "\n";
    return o.str();
}

std::string render(const Snapshot& s, const SystemInfo& info,
                   const std::vector<std::string>& alerts, bool clearScreen, bool showCores) {
    std::ostringstream o;
    char buf[256];
    if (clearScreen) o << "\033[H\033[J";

    o << BOLD << "Linux System Resource Monitor" << RESET << "   " << s.time << "   (Ctrl+C to quit)\n\n";
    o << renderSystemInfo(info) << "\n";

    o << BOLD << CYN << "== CPU ==" << RESET << "\n";
    o << "Total   " << pctBar(s.cpuTotal) << "\n";
    if (showCores) {
        for (size_t c = 0; c < s.cores.size(); ++c) {
            std::snprintf(buf, sizeof buf, "Core %-3zu", c);
            o << buf << pctBar(s.cores[c]) << "\n";
        }
    }
    std::snprintf(buf, sizeof buf, "Load avg: %.2f %.2f %.2f   Context switches: %.0f/s\n\n",
                  s.load1, s.load5, s.load15, s.ctxPerSec);
    o << buf;

    o << BOLD << CYN << "== Memory ==" << RESET << "\n";
    o << "RAM     " << pctBar(s.memPct) << "  " << humanBytes(static_cast<double>(s.memUsed)) << " / "
      << humanBytes(static_cast<double>(s.memTotal)) << "\n";
    if (s.swapTotal)
        o << "Swap    " << pctBar(s.swapPct) << "  " << humanBytes(static_cast<double>(s.swapUsed)) << " / "
          << humanBytes(static_cast<double>(s.swapTotal)) << "\n";
    o << "\n";

    o << BOLD << CYN << "== Disk & Network ==" << RESET << "\n";
    o << "Disk /  " << pctBar(s.diskPct) << "  " << humanBytes(static_cast<double>(s.diskUsed)) << " / "
      << humanBytes(static_cast<double>(s.diskTotal)) << "\n";
    o << "Disk IO : read " << humanBytes(s.diskReadBps) << "/s, write " << humanBytes(s.diskWriteBps) << "/s\n";
    o << "Network : down " << humanBytes(s.netRxBps) << "/s, up " << humanBytes(s.netTxBps) << "/s\n\n";

    o << BOLD << CYN << "== Top Processes (" << s.procCount << " total) ==" << RESET << "\n";
    std::snprintf(buf, sizeof buf, "%-8s %-20s %-5s %7s %12s\n", "PID", "NAME", "STATE", "CPU%", "MEMORY");
    o << BOLD << buf << RESET;
    for (const auto& p : s.top) {
        std::snprintf(buf, sizeof buf, "%-8d %-20.20s %-5c %7.1f %12s\n", p.pid, p.name.c_str(), p.state,
                      p.cpu, humanBytes(static_cast<double>(p.rss)).c_str());
        o << buf;
    }

    if (!alerts.empty()) {
        o << "\n";
        for (const auto& a : alerts) o << BOLD << RED << "[ALERT] " << a << RESET << "\n";
    }
    return o.str();
}

} // namespace sysmon
