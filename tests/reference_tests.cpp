#include <cstddef>
#include <type_traits>
#include <variant>

#include <gtest/gtest.h>

#include "aegisx/reference_engine.hpp"
#include "aegisx/matching_engine.hpp"
#include "aegisx/simulation.hpp"

namespace {

using namespace aegisx;

void apply_reference(ReferenceEngine& engine, const SimulationEvent& event) {
    std::visit([&](const auto& typed_event) {
        using EventType = std::decay_t<decltype(typed_event)>;
        if constexpr (std::is_same_v<EventType, SubmitEvent>) {
            engine.submit(typed_event.order);
        } else if constexpr (std::is_same_v<EventType, CancelEvent>) {
            engine.cancel(typed_event.order_id);
        } else {
            engine.modify(typed_event.order_id, typed_event.new_quantity, typed_event.new_price);
        }
    }, event);
}

void expect_same_trade(const Trade& expected, const Trade& actual) {
    EXPECT_EQ(expected.instrument_id, actual.instrument_id);
    EXPECT_EQ(expected.maker_order_id, actual.maker_order_id);
    EXPECT_EQ(expected.taker_order_id, actual.taker_order_id);
    EXPECT_EQ(expected.price, actual.price);
    EXPECT_EQ(expected.quantity, actual.quantity);
}

TEST(ReferenceEngine, MatchesTheCoreEngineOnACompleteScenario) {
    MatchingEngine core{1};
    ReferenceEngine reference{1};
    OrderGenerator generator{2026};

    for (std::size_t index = 0; index < 1000; ++index) {
        const SimulationEvent event = generator.next(core.book());
        std::vector<Trade> core_trades;
        std::visit([&](const auto& typed_event) {
            using EventType = std::decay_t<decltype(typed_event)>;
            if constexpr (std::is_same_v<EventType, SubmitEvent>) {
                core_trades = core.submit(typed_event.order);
                const std::vector<Trade> reference_trades = reference.submit(typed_event.order);
                ASSERT_EQ(core_trades.size(), reference_trades.size());
                for (std::size_t trade_index = 0; trade_index < core_trades.size(); ++trade_index) {
                    expect_same_trade(reference_trades[trade_index], core_trades[trade_index]);
                }
            } else {
                apply_reference(reference, event);
                if constexpr (std::is_same_v<EventType, CancelEvent>) {
                    core.cancel(typed_event.order_id);
                } else {
                    core.modify(typed_event.order_id, typed_event.new_quantity, typed_event.new_price);
                }
            }
        }, event);
    }

    ASSERT_EQ(core.book().order_count(), reference.orders().size());
    for (const Order& reference_order : reference.orders()) {
        const Order* core_order = core.book().find_order(reference_order.id());
        ASSERT_NE(core_order, nullptr);
        EXPECT_EQ(core_order->side(), reference_order.side());
        EXPECT_EQ(core_order->price(), reference_order.price());
        EXPECT_EQ(core_order->original_quantity(), reference_order.original_quantity());
        EXPECT_EQ(core_order->executed_quantity(), reference_order.executed_quantity());
        EXPECT_EQ(core_order->remaining_quantity(), reference_order.remaining_quantity());
    }
}

TEST(ReferenceEngine, HandlesPriceTimePriorityClearly) {
    ReferenceEngine engine{1};
    engine.submit(Order{1, 1, Side::Sell, OrderType::Limit, 50, Price{100}});
    engine.submit(Order{2, 1, Side::Sell, OrderType::Limit, 50, Price{101}});

    const std::vector<Trade> trades =
        engine.submit(Order{3, 1, Side::Buy, OrderType::Limit, 75, Price{101}});

    ASSERT_EQ(trades.size(), 2U);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 50U);
    EXPECT_EQ(trades[1].price, 101);
    EXPECT_EQ(trades[1].quantity, 25U);
}

}  // namespace
