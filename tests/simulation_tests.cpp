#include <cstddef>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>

#include <gtest/gtest.h>

#include "aegisx/invariants.hpp"
#include "aegisx/matching_engine.hpp"
#include "aegisx/simulation.hpp"

namespace {

using namespace aegisx;

std::string describe(const SimulationEvent& event) {
    std::ostringstream output;
    std::visit([&](const auto& typed_event) {
        using EventType = std::decay_t<decltype(typed_event)>;
        if constexpr (std::is_same_v<EventType, SubmitEvent>) {
            const Order& order = typed_event.order;
            output << "submit:" << order.id() << ':' << static_cast<int>(order.side()) << ':'
                   << static_cast<int>(order.type()) << ':' << order.original_quantity() << ':';
            if (order.price().has_value()) {
                output << order.price().value();
            }
        } else if constexpr (std::is_same_v<EventType, CancelEvent>) {
            output << "cancel:" << typed_event.order_id;
        } else {
            output << "modify:" << typed_event.order_id << ':' << typed_event.new_quantity << ':';
            if (typed_event.new_price.has_value()) {
                output << typed_event.new_price.value();
            }
        }
    }, event);
    return output.str();
}

void apply(MatchingEngine& engine, const SimulationEvent& event) {
    std::visit([&](const auto& typed_event) {
        using EventType = std::decay_t<decltype(typed_event)>;
        if constexpr (std::is_same_v<EventType, SubmitEvent>) {
            engine.submit_events(typed_event.order);
        } else if constexpr (std::is_same_v<EventType, CancelEvent>) {
            engine.cancel(typed_event.order_id);
        } else {
            engine.modify(typed_event.order_id, typed_event.new_quantity, typed_event.new_price);
        }
    }, event);
}

TEST(Simulation, SameSeedProducesIdenticalEventSequence) {
    MatchingEngine first_engine{1};
    MatchingEngine second_engine{1};
    OrderGenerator first_generator{42};
    OrderGenerator second_generator{42};

    for (std::size_t index = 0; index < 200; ++index) {
        const SimulationEvent first = first_generator.next(first_engine.book());
        const SimulationEvent second = second_generator.next(second_engine.book());
        EXPECT_EQ(describe(first), describe(second));
        apply(first_engine, first);
        apply(second_engine, second);
    }
}

TEST(Simulation, GeneratedEventsRemainApplicableToCurrentBookState) {
    MatchingEngine engine{1};
    OrderGenerator generator{1234};

    for (std::size_t index = 0; index < 500; ++index) {
        const SimulationEvent event = generator.next(engine.book());
        EXPECT_NO_THROW(apply(engine, event));
        EXPECT_TRUE(InvariantChecker::check_book(engine.book()).valid());
    }
}

}  // namespace
