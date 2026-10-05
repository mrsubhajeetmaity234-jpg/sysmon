#include "logger.hpp"
#include <filesystem>

namespace sysmon {

CsvLogger::CsvLogger(const std::string& path) {
    bool needHeader = !std::filesystem::exists(path) || std::filesystem::file_size(path) == 0;
    out_.open(path, std::ios::app);
    if (out_ && needHeader)
        out_ << "time,cpu_pct,load1,load5,load15,ctx_per_s,mem_used_bytes,mem_pct,swap_pct,"
                "disk_pct,disk_read_Bps,disk_write_Bps,net_rx_Bps,net_tx_Bps,processes\n";
}

void CsvLogger::write(const Snapshot& s) {
    if (!out_) return;
    out_ << s.time << ',' << s.cpuTotal << ',' << s.load1 << ',' << s.load5 << ',' << s.load15 << ','
         << s.ctxPerSec << ',' << s.memUsed << ',' << s.memPct << ',' << s.swapPct << ','
         << s.diskPct << ',' << s.diskReadBps << ',' << s.diskWriteBps << ','
         << s.netRxBps << ',' << s.netTxBps << ',' << s.procCount << '\n';
    out_.flush();
}

} // namespace sysmon
