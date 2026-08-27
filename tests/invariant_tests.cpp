#include <gtest/gtest.h>

#include "aegisx/invariants.hpp"
#include "aegisx/matching_engine.hpp"

namespace {

using namespace aegisx;

TEST(Invariants, ValidatesNewAndPartiallyFilledOrders) {
    Order order{1, 7, Side::Buy, OrderType::Limit, 100, Price{100}};
    EXPECT_TRUE(InvariantChecker::check_order(order).valid());

    order.execute(40);
    const InvariantReport report = InvariantChecker::check_order(order);
    EXPECT_TRUE(report.valid());
    EXPECT_EQ(order.executed_quantity() + order.remaining_quantity(), 100U);
}

TEST(Invariants, AllowsValidCancelledOrderState) {
    Order order{1, 7, Side::Sell, OrderType::Limit, 100, Price{100}};
    order.execute(25);
    order.cancel();

    EXPECT_TRUE(InvariantChecker::check_order(order).valid());
}

TEST(Invariants, DetectsInvalidOrderBookStateByCheckingLiveBook) {
    MatchingEngine engine{7};
    engine.submit(Order{1, 7, Side::Sell, OrderType::Limit, 100, Price{100}});
    engine.submit(Order{2, 7, Side::Buy, OrderType::Limit, 40, Price{99}});

    const InvariantReport report = InvariantChecker::check_book(engine.book());
    EXPECT_TRUE(report.valid());
    EXPECT_TRUE(report.violations.empty());
}

TEST(Invariants, RemovalsAndFillsLeaveTheBookValid) {
    MatchingEngine engine{7};
    engine.submit(Order{1, 7, Side::Sell, OrderType::Limit, 100, Price{100}});
    engine.submit(Order{2, 7, Side::Buy, OrderType::Limit, 40, Price{100}});
    EXPECT_TRUE(InvariantChecker::check_book(engine.book()).valid());

    engine.cancel(1);
    EXPECT_TRUE(InvariantChecker::check_book(engine.book()).valid());
}

TEST(Invariants, LocationIndexTracksOrdersAcrossPriceLevels) {
    MatchingEngine engine{7};
    engine.submit(Order{1, 7, Side::Buy, OrderType::Limit, 10, Price{101}});
    engine.submit(Order{2, 7, Side::Buy, OrderType::Limit, 10, Price{99}});
    engine.submit(Order{3, 7, Side::Sell, OrderType::Limit, 10, Price{103}});

    EXPECT_TRUE(InvariantChecker::check_book(engine.book()).valid());
    ASSERT_FALSE(engine.modify(2, 5, Price{100}).empty());
    EXPECT_TRUE(InvariantChecker::check_book(engine.book()).valid());

    ASSERT_FALSE(engine.cancel(1).empty());
    EXPECT_TRUE(InvariantChecker::check_book(engine.book()).valid());
}

}  // namespace
