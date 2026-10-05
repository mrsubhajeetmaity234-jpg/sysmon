#pragma once
#include <string>

namespace sysmon {
// Abstract base class: every resource monitor is sampled the same way (polymorphism).
class Monitor {
public:
    virtual ~Monitor() = default;
    virtual void sample() = 0;               // read fresh data from /proc or /sys
    virtual std::string name() const = 0;
};
}
