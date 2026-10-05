# Linux System Resource Monitor

![Language](https://img.shields.io/badge/C%2B%2B-17-blue)
![Platform](https://img.shields.io/badge/platform-Linux-green)
![License](https://img.shields.io/badge/license-MIT-lightgrey)
<!-- After pushing, add the CI badge: ![CI](https://github.com/<mrsubhajeetmaity234-jpg>/sysmon/actions/workflows/ci.yml/badge.svg) -->

A multithreaded, terminal-based system monitor written in pure C++17 for Linux.
It reads live data directly from the kernel (`/proc`, `/sys`) and shows CPU, memory, disk,
network and process statistics, raises threshold alerts, and logs everything to CSV.
No external libraries required.

## Features
- Total and per-core CPU usage, load average, context switches per second
- RAM and swap usage, disk space and I/O throughput, network download/upload speed
- Top-N processes sorted by CPU or memory
- Architecture panel: CPU model, core count, endianness, page size, L1/L2/L3 cache sizes
- Configurable alerts for CPU, memory and disk usage
- CSV logging for later analysis

See [docs/sample_output.txt](docs/sample_output.txt) for a sample run.

## Requirements
Linux, `g++` 8 or newer (C++17), `make`.

## Build and run
```bash
git clone https://github.com/<mrsubhajeetmaity234-jpg>/sysmon.git
cd sysmon
make            # builds bin/sysmon
make run        # build and run
make test       # smoke tests
make clean
```

### Options
| Option | Description |
|---|---|
| `-i <sec>` | refresh interval (default 1) |
| `-n <count>` | stop after N samples (default: run until Ctrl+C) |
| `-t <num>` | number of top processes (default 8) |
| `-s cpu\|mem` | sort processes by CPU or memory |
| `-l <file>` | CSV log file (default `logs/sysmon.csv`) |
| `--no-log`, `--no-clear`, `--no-cores` | disable logging / screen clearing / per-core bars |
| `--cpu-alert`, `--mem-alert`, `--disk-alert <pct>` | alert thresholds (default 90) |
| `-h` | help |

Example: `./bin/sysmon -i 2 -t 10 -s mem --cpu-alert 80`

## Architecture
```
 /proc, /sys
      |
      v
 +--------------------------+
 | Monitor classes          |   CpuMonitor, MemoryMonitor, DiskMonitor,
 | (derived from Monitor)   |   NetworkMonitor, ProcessMonitor
 +------------+-------------+
              |  sample()
              v
 +--------------------------+
 | Sampler thread           |   builds a Snapshot every N seconds
 +------+-------------+-----+
        |             |
        v             v
  SafeQueue<Snapshot> x2   (mutex + condition variable)
        |             |
        v             v
 +-------------+  +---------------+
 | Main thread |  | Logger thread |
 | alerts +    |  | CSV writer    |
 | dashboard   |  |               |
 +-------------+  +---------------+
```

## Project layout
```
include/     headers                      src/        implementations + main.cpp
tests/       smoke test                   docs/       sample output
.github/     CI workflow                  Makefile    build script
```

| Module | Responsibility | Source |
|---|---|---|
| `CpuMonitor` | usage %, load average, context switches | `/proc/stat`, `/proc/loadavg` |
| `MemoryMonitor` | RAM and swap | `/proc/meminfo` |
| `DiskMonitor` | space and I/O rate | `statvfs()`, `/proc/diskstats` |
| `NetworkMonitor` | rx/tx rate | `/proc/net/dev` |
| `ProcessMonitor` | per-process CPU % and memory | `/proc/<pid>/stat`, `statm` |
| `SystemInfo` | architecture facts | `uname()`, `sysconf()`, `/proc/cpuinfo`, `/sys` |
| `SafeQueue<T>` | thread-safe producer/consumer queue | `std::mutex`, `std::condition_variable` |

## Concepts demonstrated
- **Linux:** `/proc` and `/sys` virtual filesystems, system calls, signals, Makefile, Git, CI
- **C++:** inheritance and virtual functions, RAII, templates, STL, smart pointers, lambdas, `std::thread`, `std::filesystem`, `std::chrono`
- **Computer architecture:** CPU time accounting, context switches, cache hierarchy, page size and virtual memory, endianness, disk sectors
- **OS concepts:** processes and states, scheduling, synchronisation, producer-consumer problem

## How the calculations work
- **CPU %** = `100 * (1 - dIdle / dTotal)`, using the difference between two reads of `/proc/stat` (the counters are cumulative since boot).
- **Per-process CPU %** = `(delta(utime + stime) / CLK_TCK) / elapsed seconds * 100`.
- **Rates** (disk, network, context switches) = change in a cumulative counter divided by elapsed time.
- **Process memory (RSS)** = resident pages from `/proc/<pid>/statm` times the page size.

## Limitations
Linux only. Containers and VMs may show limited disk and cache information.

## License
MIT. See [LICENSE](LICENSE).
