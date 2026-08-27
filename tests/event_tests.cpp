#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "aegisx/event.hpp"
#include "aegisx/matching_engine.hpp"

namespace {

using namespace aegisx;

TEST(Events, EmitAcceptanceTradeAndRestSequence) {
    MatchingEngine engine{7};
    engine.submit(Order{1, 7, Side::Sell, OrderType::Limit, 100, Price{100}});

    const std::vector<Event> events =
        engine.submit_events(Order{2, 7, Side::Buy, OrderType::Limit, 150, Price{101}});

    ASSERT_EQ(events.size(), 3U);
    ASSERT_TRUE(std::holds_alternative<OrderAccepted>(events[0]));
    ASSERT_TRUE(std::holds_alternative<TradeExecuted>(events[1]));
    ASSERT_TRUE(std::holds_alternative<OrderRested>(events[2]));

    const OrderAccepted& accepted = std::get<OrderAccepted>(events[0]);
    EXPECT_EQ(accepted.order_id, 2U);
    EXPECT_EQ(accepted.quantity, 150U);

    const TradeExecuted& executed = std::get<TradeExecuted>(events[1]);
    EXPECT_EQ(executed.trade.maker_order_id, 1U);
    EXPECT_EQ(executed.trade.taker_order_id, 2U);
    EXPECT_EQ(executed.trade.quantity, 100U);

    const OrderRested& rested = std::get<OrderRested>(events[2]);
    EXPECT_EQ(rested.order_id, 2U);
    EXPECT_EQ(rested.price, 101);
    EXPECT_EQ(rested.remaining_quantity, 50U);
}

TEST(Events, MarketOrderProducesAcceptanceAndTradesButNeverRests) {
    MatchingEngine engine{7};
    engine.submit(Order{1, 7, Side::Sell, OrderType::Limit, 25, Price{100}});

    const std::vector<Event> events =
        engine.submit_events(Order{2, 7, Side::Buy, OrderType::Market, 50});

    ASSERT_EQ(events.size(), 2U);
    EXPECT_TRUE(std::holds_alternative<OrderAccepted>(events[0]));
    EXPECT_TRUE(std::holds_alternative<TradeExecuted>(events[1]));
    EXPECT_TRUE(engine.book().empty());
}

TEST(Events, TradeOnlyCompatibilityApiRemainsAvailable) {
    MatchingEngine engine{7};
    engine.submit(Order{1, 7, Side::Sell, OrderType::Limit, 10, Price{100}});

    const std::vector<Trade> trades =
        engine.submit(Order{2, 7, Side::Buy, OrderType::Limit, 10, Price{100}});

    ASSERT_EQ(trades.size(), 1U);
    EXPECT_EQ(trades[0].quantity, 10U);
}

}  // namespace
