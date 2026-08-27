#include <cstddef>
#include <exception>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

#include "aegisx/matching_engine.hpp"
#include "aegisx/replay.hpp"

namespace {

using namespace aegisx;

int run(const std::string& path) {
    const std::vector<SimulationEvent> events = EventLog::read_file(path);
    MatchingEngine engine{1};
    std::size_t trades = 0;
    for (const SimulationEvent& event : events) {
        std::visit([&](const auto& typed_event) {
            using EventType = std::decay_t<decltype(typed_event)>;
            if constexpr (std::is_same_v<EventType, SubmitEvent>) {
                for (const Event& output : engine.submit_events(typed_event.order)) {
                    if (std::holds_alternative<TradeExecuted>(output)) {
                        ++trades;
                    }
                }
            } else if constexpr (std::is_same_v<EventType, CancelEvent>) {
                engine.cancel(typed_event.order_id);
            } else {
                engine.modify(typed_event.order_id, typed_event.new_quantity, typed_event.new_price);
            }
        }, event);
    }
    std::cout << "input_events=" << events.size()
              << " trades=" << trades
              << " resting_orders=" << engine.book().order_count() << '\n';
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 3 || std::string{argv[1]} != "--input") {
        std::cerr << "usage: aegisx-replay --input FILE\n";
        return 1;
    }
    try {
        return run(argv[2]);
    } catch (const std::exception& error) {
        std::cerr << "replay error: " << error.what() << '\n';
        return 1;
    }
}
