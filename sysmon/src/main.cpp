// Linux System Resource Monitor
//
// Threads:
//   sampler thread : reads /proc every N seconds, builds a Snapshot, pushes it to two queues
//   logger  thread : pops Snapshots and appends them to a CSV file
//   main    thread : pops Snapshots, checks alerts, prints the dashboard
#include "alert.hpp"
#include "cpu_monitor.hpp"
#include "disk_monitor.hpp"
#include "display.hpp"
#include "logger.hpp"
#include "memory_monitor.hpp"
#include "network_monitor.hpp"
#include "process_monitor.hpp"
#include "safe_queue.hpp"
#include "snapshot.hpp"
#include "sysinfo.hpp"
#include "utils.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace sysmon;

static std::atomic<bool> g_stop{false};
static void onSignal(int) { g_stop = true; }

struct Options {
    double interval = 1.0;
    long count = 0;               // 0 = run until Ctrl+C
    std::string logFile = "logs/sysmon.csv";
    bool log = true;
    bool clear = true;
    bool cores = true;
    size_t topN = 8;
    SortKey sort = SortKey::Cpu;
    Thresholds th;
};

static void usage(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
              << "  -i <sec>     refresh interval (default 1)\n"
              << "  -n <count>   stop after <count> samples (default: run forever)\n"
              << "  -t <num>     number of top processes to show (default 8)\n"
              << "  -s cpu|mem   sort processes by CPU or memory (default cpu)\n"
              << "  -l <file>    CSV log file (default logs/sysmon.csv)\n"
              << "  --no-log     disable CSV logging\n"
              << "  --no-clear   do not clear the screen between refreshes\n"
              << "  --no-cores   hide per-core bars\n"
              << "  --cpu-alert <pct>  --mem-alert <pct>  --disk-alert <pct>   alert thresholds (default 90)\n"
              << "  -h           show this help\n";
}

static bool parseArgs(int argc, char** argv, Options& o) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&]() -> const char* { return (i + 1 < argc) ? argv[++i] : nullptr; };
        const char* v = nullptr;
        if (a == "-h" || a == "--help") return false;
        else if (a == "-i" && (v = next())) o.interval = std::atof(v);
        else if (a == "-n" && (v = next())) o.count = std::atol(v);
        else if (a == "-t" && (v = next())) o.topN = static_cast<size_t>(std::atol(v));
        else if (a == "-s" && (v = next())) o.sort = (std::string(v) == "mem") ? SortKey::Memory : SortKey::Cpu;
        else if (a == "-l" && (v = next())) o.logFile = v;
        else if (a == "--no-log") o.log = false;
        else if (a == "--no-clear") o.clear = false;
        else if (a == "--no-cores") o.cores = false;
        else if (a == "--cpu-alert" && (v = next())) o.th.cpu = std::atof(v);
        else if (a == "--mem-alert" && (v = next())) o.th.mem = std::atof(v);
        else if (a == "--disk-alert" && (v = next())) o.th.disk = std::atof(v);
        else { std::cerr << "Unknown or incomplete option: " << a << "\n"; return false; }
    }
    return o.interval > 0;
}

int main(int argc, char** argv) {
    Options opt;
    if (!parseArgs(argc, argv, opt)) { usage(argv[0]); return 1; }

    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    const SystemInfo info = SystemInfo::collect();

    // Polymorphism: hold all monitors through the common base class.
    CpuMonitor cpu;
    MemoryMonitor mem;
    DiskMonitor disk("/");
    NetworkMonitor net;
    ProcessMonitor procs;
    std::vector<Monitor*> monitors{&cpu, &mem, &disk, &net, &procs};

    // Baseline reading so the first displayed values are real deltas.
    for (auto* m : monitors) m->sample();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    SafeQueue<Snapshot> displayQ, logQ;

    std::unique_ptr<CsvLogger> logger;
    if (opt.log) {
        logger = std::make_unique<CsvLogger>(opt.logFile);
        if (!logger->ok()) {
            std::cerr << "Warning: cannot open log file '" << opt.logFile << "' (logging disabled)\n";
            logger.reset();
        }
    }

    // ---- sampler thread (producer) ----
    std::thread sampler([&] {
        auto next = std::chrono::steady_clock::now();
        while (!g_stop) {
            for (auto* m : monitors) m->sample();

            Snapshot s;
            s.time = timestamp();
            s.cpuTotal = cpu.totalUsage();
            s.cores = cpu.coreUsage();
            s.load1 = cpu.load1(); s.load5 = cpu.load5(); s.load15 = cpu.load15();
            s.ctxPerSec = cpu.ctxSwitchesPerSec();
            s.memTotal = mem.total(); s.memUsed = mem.used(); s.memPct = mem.usedPercent();
            s.swapTotal = mem.swapTotal(); s.swapUsed = mem.swapUsed(); s.swapPct = mem.swapPercent();
            s.diskTotal = disk.total(); s.diskUsed = disk.used(); s.diskPct = disk.usedPercent();
            s.diskReadBps = disk.readBytesPerSec(); s.diskWriteBps = disk.writeBytesPerSec();
            s.netRxBps = net.rxBytesPerSec(); s.netTxBps = net.txBytesPerSec();
            s.procCount = procs.count();
            s.top = procs.top(opt.topN, opt.sort);

            if (logger) logQ.push(s);
            displayQ.push(std::move(s));

            // sleep until next tick in small slices so Ctrl+C reacts quickly
            next += std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>(opt.interval));
            while (!g_stop && std::chrono::steady_clock::now() < next)
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        displayQ.close();
        logQ.close();
    });

    // ---- logger thread (consumer 1) ----
    std::thread logThread([&] {
        Snapshot s;
        while (logQ.pop(s)) logger->write(s);
    });
    if (!logger) logQ.close();   // nothing to log: let the thread exit immediately

    // ---- main thread (consumer 2): display ----
    Snapshot s;
    long shown = 0;
    while (displayQ.pop(s)) {
        auto alerts = checkAlerts(s, opt.th);
        std::cout << render(s, info, alerts, opt.clear, opt.cores) << std::flush;
        if (!opt.clear) std::cout << "\n";
        if (opt.count > 0 && ++shown >= opt.count) g_stop = true;
    }

    sampler.join();
    logThread.join();
    if (opt.log && logger) std::cout << "\nLog saved to " << opt.logFile << "\n";
    std::cout << "Monitor stopped.\n";
    return 0;
}
