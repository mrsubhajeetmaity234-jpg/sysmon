#pragma once
#include "snapshot.hpp"
#include <fstream>
#include <string>

namespace sysmon {

// Appends one CSV row per snapshot (RAII: file closed in the destructor).
class CsvLogger {
public:
    explicit CsvLogger(const std::string& path);
    bool ok() const { return out_.is_open(); }
    void write(const Snapshot& s);

private:
    std::ofstream out_;
};
}
