#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <variant>

#include "aegisx/matching_engine.hpp"
#include "aegisx/simulation.hpp"

namespace {

using namespace aegisx;

struct Counters {
    std::size_t submissions{0};
    std::size_t cancellations{0};
    std::size_t modifications{0};
    std::size_t trades{0};
};

void apply(MatchingEngine& engine, const SimulationEvent& event, Counters& counters) {
    std::visit([&](const auto& typed_event) {
        using EventType = std::decay_t<decltype(typed_event)>;
        if constexpr (std::is_same_v<EventType, SubmitEvent>) {
            const auto events = engine.submit_events(typed_event.order);
            for (const Event& output : events) {
                if (std::holds_alternative<TradeExecuted>(output)) {
                    ++counters.trades;
                }
            }
            ++counters.submissions;
        } else if constexpr (std::is_same_v<EventType, CancelEvent>) {
            engine.cancel(typed_event.order_id);
            ++counters.cancellations;
        } else {
            engine.modify(typed_event.order_id, typed_event.new_quantity, typed_event.new_price);
            ++counters.modifications;
        }
    }, event);
}

bool parse_unsigned(std::string_view value, std::uint64_t& result) {
    try {
        std::size_t parsed = 0;
        const std::string text{value};
        result = std::stoull(text, &parsed);
        return parsed == text.size();
    } catch (const std::exception&) {
        return false;
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::uint64_t order_count = 1000;
    std::uint64_t seed = 42;
    for (int index = 1; index < argc; ++index) {
        const std::string_view argument{argv[index]};
        if (argument == "--orders" && index + 1 < argc) {
            if (!parse_unsigned(argv[++index], order_count)) {
                std::cerr << "invalid --orders value\n";
                return 1;
            }
        } else if (argument == "--seed" && index + 1 < argc) {
            if (!parse_unsigned(argv[++index], seed)) {
                std::cerr << "invalid --seed value\n";
                return 1;
            }
        } else {
            std::cerr << "usage: aegisx-sim [--orders N] [--seed N]\n";
            return 1;
        }
    }

    MatchingEngine engine{1};
    OrderGenerator generator{seed, 1};
    Counters counters;
    for (std::uint64_t index = 0; index < order_count; ++index) {
        apply(engine, generator.next(engine.book()), counters);
    }

    std::cout << "seed=" << seed << " events=" << order_count
              << " submissions=" << counters.submissions
              << " cancellations=" << counters.cancellations
              << " modifications=" << counters.modifications
              << " trades=" << counters.trades
              << " resting_orders=" << engine.book().order_count() << '\n';
    return 0;
}
