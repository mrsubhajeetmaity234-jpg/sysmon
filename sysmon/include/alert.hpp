#pragma once
#include "snapshot.hpp"
#include <string>
#include <vector>

namespace sysmon {

struct Thresholds {
    double cpu = 90.0;
    double mem = 90.0;
    double disk = 90.0;
};

// Returns a human-readable warning for each metric above its threshold.
std::vector<std::string> checkAlerts(const Snapshot& s, const Thresholds& t);
}
