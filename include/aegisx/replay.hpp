#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "aegisx/simulation.hpp"

namespace aegisx {

class EventLog {
public:
    static void write(std::ostream& output, const std::vector<SimulationEvent>& events);
    [[nodiscard]] static std::vector<SimulationEvent> read(std::istream& input);
    static void write_file(const std::string& path, const std::vector<SimulationEvent>& events);
    [[nodiscard]] static std::vector<SimulationEvent> read_file(const std::string& path);
};

}  // namespace aegisx
