#include "sysinfo.hpp"
#include "utils.hpp"
#include <cstdint>
#include <sstream>
#include <sys/utsname.h>
#include <unistd.h>

namespace sysmon {

SystemInfo SystemInfo::collect() {
    SystemInfo si;

    utsname u{};
    if (uname(&u) == 0) {
        si.hostname = u.nodename;
        si.kernel = std::string(u.sysname) + " " + u.release;
        si.arch = u.machine;
    }
    si.logicalCores = sysconf(_SC_NPROCESSORS_ONLN);
    si.pageSize = sysconf(_SC_PAGESIZE);
    si.totalRam = static_cast<unsigned long long>(sysconf(_SC_PHYS_PAGES)) * si.pageSize;

    // Endianness: look at the first byte of a 16-bit value 1.
    std::uint16_t probe = 1;
    si.littleEndian = (*reinterpret_cast<unsigned char*>(&probe) == 1);

    // CPU model name from /proc/cpuinfo (x86 "model name", ARM "Model"/"Hardware")
    std::string text;
    if (readFile("/proc/cpuinfo", text)) {
        std::istringstream in(text);
        std::string line;
        while (std::getline(in, line)) {
            if (line.rfind("model name", 0) == 0 || line.rfind("Model", 0) == 0 ||
                line.rfind("Hardware", 0) == 0) {
                auto p = line.find(':');
                if (p != std::string::npos) { si.cpuModel = trim(line.substr(p + 1)); break; }
            }
        }
    }
    if (si.cpuModel.empty()) si.cpuModel = "Unknown";

    // Cache hierarchy of core 0 from sysfs
    for (int i = 0; i < 10; ++i) {
        std::string base = "/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(i) + "/";
        std::string level, type, size;
        if (!readFile(base + "level", level)) break;
        readFile(base + "type", type);
        readFile(base + "size", size);
        si.caches.push_back("L" + trim(level) + " " + trim(type) + " " + trim(size));
    }
    return si;
}

} // namespace sysmon
