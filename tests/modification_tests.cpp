#include <stdexcept>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "aegisx/event.hpp"
#include "aegisx/matching_engine.hpp"

namespace {

using namespace aegisx;

Order buy(OrderId id, Quantity quantity, Price price) {
    return Order{id, 7, Side::Buy, OrderType::Limit, quantity, price};
}

TEST(Modification, QuantityDecreasePreservesTimePriority) {
    MatchingEngine engine{7};
    engine.submit(buy(1, 100, 100));
    engine.submit(buy(2, 100, 100));

    const std::vector<Event> events = engine.modify(1, 60);

    ASSERT_TRUE(std::holds_alternative<OrderModified>(events.front()));
    EXPECT_FALSE(std::get<OrderModified>(events.front()).priority_reset);
    ASSERT_NE(engine.book().best_bid(), nullptr);
    EXPECT_EQ(engine.book().best_bid()->front().id(), 1U);
    EXPECT_EQ(engine.book().best_bid()->front().remaining_quantity(), 60U);
}

TEST(Modification, QuantityIncreaseLosesTimePriority) {
    MatchingEngine engine{7};
    engine.submit(buy(1, 100, 100));
    engine.submit(buy(2, 100, 100));

    const std::vector<Event> events = engine.modify(1, 150);

    ASSERT_TRUE(std::get<OrderModified>(events.front()).priority_reset);
    ASSERT_NE(engine.book().best_bid(), nullptr);
    EXPECT_EQ(engine.book().best_bid()->front().id(), 2U);
    EXPECT_EQ(engine.book().best_bid()->orders().at(1).id(), 1U);
}

TEST(Modification, PriceChangeReinsertsAtNewPrice) {
    MatchingEngine engine{7};
    engine.submit(buy(1, 100, 100));
    engine.submit(buy(2, 100, 101));

    const std::vector<Event> events = engine.modify(1, 100, Price{102});

    const OrderModified& modified = std::get<OrderModified>(events.front());
    EXPECT_TRUE(modified.priority_reset);
    EXPECT_EQ(modified.price, 102);
    ASSERT_NE(engine.book().best_bid(), nullptr);
    EXPECT_EQ(engine.book().best_bid()->price(), 102);
    EXPECT_EQ(engine.book().best_bid()->front().id(), 1U);
    EXPECT_EQ(engine.book().bid_level_count(), 2U);
}

TEST(Modification, QuantityDecreaseAfterPartialFillPreservesExecutedQuantity) {
    MatchingEngine engine{7};
    engine.submit(Order{1, 7, Side::Sell, OrderType::Limit, 100, Price{100}});
    engine.submit(buy(2, 30, 100));

    const std::vector<Event> events = engine.modify(1, 70);

    const OrderModified& modified = std::get<OrderModified>(events.front());
    EXPECT_EQ(modified.original_quantity, 70U);
    EXPECT_EQ(modified.remaining_quantity, 40U);
    ASSERT_NE(engine.book().best_ask(), nullptr);
    EXPECT_EQ(engine.book().best_ask()->front().remaining_quantity(), 40U);
}

TEST(Modification, RejectsInvalidChangesWithoutRemovingOrder) {
    MatchingEngine engine{7};
    engine.submit(buy(1, 100, 100));

    EXPECT_THROW(engine.modify(99, 10), std::invalid_argument);
    EXPECT_THROW(engine.modify(1, 0), std::invalid_argument);
    EXPECT_THROW(engine.modify(1, 101, Price{0}), std::invalid_argument);
    EXPECT_TRUE(engine.book().contains_order(1));

    engine.submit(Order{2, 7, Side::Sell, OrderType::Limit, 50, Price{100}});
    EXPECT_THROW(engine.modify(1, 20), std::invalid_argument);
}

TEST(Modification, FilledAndCancelledOrdersCannotBeModified) {
    MatchingEngine engine{7};
    engine.submit(Order{1, 7, Side::Sell, OrderType::Limit, 10, Price{100}});
    engine.submit(buy(2, 10, 100));
    EXPECT_THROW(engine.modify(1, 20), std::invalid_argument);

    engine.submit(buy(3, 10, 101));
    engine.cancel(3);
    EXPECT_THROW(engine.modify(3, 20), std::invalid_argument);
}

}  // namespace
