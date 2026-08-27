#include <sstream>
#include <type_traits>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "aegisx/invariants.hpp"
#include "aegisx/matching_engine.hpp"
#include "aegisx/replay.hpp"

namespace {

using namespace aegisx;

void apply_event(MatchingEngine& engine, const SimulationEvent& event) {
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

TEST(Replay, EventLogRoundTripIsStable) {
    MatchingEngine source_engine{1};
    OrderGenerator generator{42};
    std::vector<SimulationEvent> original;
    for (int index = 0; index < 200; ++index) {
        original.push_back(generator.next(source_engine.book()));
        apply_event(source_engine, original.back());
    }

    std::ostringstream first_output;
    EventLog::write(first_output, original);
    std::istringstream input(first_output.str());
    const std::vector<SimulationEvent> decoded = EventLog::read(input);
    std::ostringstream second_output;
    EventLog::write(second_output, decoded);

    EXPECT_EQ(first_output.str(), second_output.str());
}

TEST(Replay, ReplayingRecordedEventsReproducesFinalBook) {
    MatchingEngine original_engine{1};
    MatchingEngine replay_engine{1};
    OrderGenerator generator{99};
    std::vector<SimulationEvent> events;
    for (int index = 0; index < 300; ++index) {
        events.push_back(generator.next(original_engine.book()));
        apply_event(original_engine, events.back());
    }

    std::ostringstream output;
    EventLog::write(output, events);
    std::istringstream input(output.str());
    const std::vector<SimulationEvent> decoded = EventLog::read(input);
    for (const SimulationEvent& event : decoded) {
        apply_event(replay_engine, event);
    }

    EXPECT_EQ(original_engine.book().order_count(), replay_engine.book().order_count());
    EXPECT_EQ(original_engine.book().bid_level_count(), replay_engine.book().bid_level_count());
    EXPECT_EQ(original_engine.book().ask_level_count(), replay_engine.book().ask_level_count());
    EXPECT_TRUE(InvariantChecker::check_book(original_engine.book()).valid());
    EXPECT_TRUE(InvariantChecker::check_book(replay_engine.book()).valid());
    if (original_engine.book().best_bid() != nullptr) {
        ASSERT_NE(replay_engine.book().best_bid(), nullptr);
        EXPECT_EQ(original_engine.book().best_bid()->price(), replay_engine.book().best_bid()->price());
    }
    if (original_engine.book().best_ask() != nullptr) {
        ASSERT_NE(replay_engine.book().best_ask(), nullptr);
        EXPECT_EQ(original_engine.book().best_ask()->price(), replay_engine.book().best_ask()->price());
    }
}

}  // namespace
