#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

#include "aegisx/event.hpp"
#include "aegisx/matching_engine.hpp"

namespace {

using namespace aegisx;

Order buy(OrderId id, Quantity quantity, Price price) {
    return Order{id, 7, Side::Buy, OrderType::Limit, quantity, price};
}

Order sell(OrderId id, Quantity quantity, Price price) {
    return Order{id, 7, Side::Sell, OrderType::Limit, quantity, price};
}

TEST(Cancellation, CancelsActiveOrderAndEmitsEvent) {
    MatchingEngine engine{7};
    engine.submit(buy(1, 100, 100));

    const std::vector<Event> events = engine.cancel(1);

    ASSERT_EQ(events.size(), 1U);
    ASSERT_TRUE(std::holds_alternative<OrderCancelled>(events[0]));
    const OrderCancelled& cancelled = std::get<OrderCancelled>(events[0]);
    EXPECT_EQ(cancelled.order_id, 1U);
    EXPECT_EQ(cancelled.side, Side::Buy);
    EXPECT_EQ(cancelled.remaining_quantity, 100U);
    EXPECT_TRUE(engine.book().empty());
    EXPECT_EQ(engine.book().bid_level_count(), 0U);
}

TEST(Cancellation, CancelsPartiallyFilledOrderWithRemainingQuantity) {
    MatchingEngine engine{7};
    engine.submit(sell(1, 100, 100));
    engine.submit(buy(2, 40, 100));

    const std::vector<Event> events = engine.cancel(1);

    ASSERT_EQ(events.size(), 1U);
    const OrderCancelled& cancelled = std::get<OrderCancelled>(events[0]);
    EXPECT_EQ(cancelled.order_id, 1U);
    EXPECT_EQ(cancelled.remaining_quantity, 60U);
    EXPECT_TRUE(engine.book().empty());
}

TEST(Cancellation, RemovesHeadMiddleAndTailWithoutChangingOtherFIFOOrders) {
    MatchingEngine engine{7};
    engine.submit(buy(1, 10, 100));
    engine.submit(buy(2, 20, 100));
    engine.submit(buy(3, 30, 100));

    engine.cancel(1);
    ASSERT_NE(engine.book().best_bid(), nullptr);
    EXPECT_EQ(engine.book().best_bid()->front().id(), 2U);

    engine.cancel(2);
    ASSERT_NE(engine.book().best_bid(), nullptr);
    EXPECT_EQ(engine.book().best_bid()->front().id(), 3U);

    engine.cancel(3);
    EXPECT_TRUE(engine.book().empty());
    EXPECT_EQ(engine.book().bid_level_count(), 0U);
}

TEST(Cancellation, CleansOnlyTheCancelledEmptyPriceLevel) {
    MatchingEngine engine{7};
    engine.submit(buy(1, 10, 101));
    engine.submit(buy(2, 10, 100));

    engine.cancel(1);

    ASSERT_NE(engine.book().best_bid(), nullptr);
    EXPECT_EQ(engine.book().best_bid()->price(), 100);
    EXPECT_EQ(engine.book().bid_level_count(), 1U);
}

TEST(Cancellation, RejectsNonexistentFilledAndAlreadyCancelledOrders) {
    MatchingEngine engine{7};
    EXPECT_THROW(engine.cancel(99), std::invalid_argument);

    engine.submit(sell(1, 10, 100));
    engine.submit(buy(2, 10, 100));
    EXPECT_THROW(engine.cancel(1), std::invalid_argument);

    engine.submit(buy(3, 10, 101));
    engine.cancel(3);
    EXPECT_THROW(engine.cancel(3), std::invalid_argument);
}

}  // namespace
