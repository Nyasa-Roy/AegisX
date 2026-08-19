#include <optional>
#include <stdexcept>

#include <gtest/gtest.h>

#include "aegisx/order.hpp"

namespace {

using namespace aegisx;

TEST(Order, CreatesLimitOrderWithIntegerPrice) {
    const Order order{1, 42, Side::Buy, OrderType::Limit, 100, Price{15025}};

    EXPECT_EQ(order.id(), 1U);
    EXPECT_EQ(order.instrument_id(), 42U);
    EXPECT_EQ(order.side(), Side::Buy);
    EXPECT_EQ(order.type(), OrderType::Limit);
    EXPECT_EQ(order.price(), std::optional<Price>{15025});
    EXPECT_EQ(order.original_quantity(), 100U);
    EXPECT_EQ(order.executed_quantity(), 0U);
    EXPECT_EQ(order.remaining_quantity(), 100U);
    EXPECT_EQ(order.status(), OrderStatus::Active);
}

TEST(Order, MarketOrderHasNoPrice) {
    const Order order{2, 42, Side::Sell, OrderType::Market, 50};

    EXPECT_EQ(order.type(), OrderType::Market);
    EXPECT_FALSE(order.price().has_value());
}

TEST(Order, ExecutionUpdatesQuantitiesAndStatus) {
    Order order{3, 42, Side::Buy, OrderType::Limit, 100, Price{10000}};

    order.execute(40);
    EXPECT_EQ(order.executed_quantity(), 40U);
    EXPECT_EQ(order.remaining_quantity(), 60U);
    EXPECT_EQ(order.status(), OrderStatus::PartiallyFilled);

    order.execute(60);
    EXPECT_EQ(order.executed_quantity(), 100U);
    EXPECT_EQ(order.remaining_quantity(), 0U);
    EXPECT_EQ(order.status(), OrderStatus::Filled);
    EXPECT_FALSE(order.is_active());
}

TEST(Order, CancelPreservesRemainingQuantity) {
    Order order{4, 42, Side::Sell, OrderType::Limit, 100, Price{10000}};
    order.execute(25);
    order.cancel();

    EXPECT_EQ(order.status(), OrderStatus::Cancelled);
    EXPECT_EQ(order.remaining_quantity(), 75U);
    EXPECT_FALSE(order.is_active());
}

TEST(Order, RejectsInvalidConstruction) {
    EXPECT_THROW(Order(0, 42, Side::Buy, OrderType::Limit, 1, Price{1}), std::invalid_argument);
    EXPECT_THROW(Order(1, 0, Side::Buy, OrderType::Limit, 1, Price{1}), std::invalid_argument);
    EXPECT_THROW(Order(1, 42, Side::Buy, OrderType::Limit, 0, Price{1}), std::invalid_argument);
    EXPECT_THROW(Order(1, 42, Side::Buy, OrderType::Limit, 1), std::invalid_argument);
    EXPECT_THROW(Order(1, 42, Side::Buy, OrderType::Limit, 1, Price{0}), std::invalid_argument);
    EXPECT_THROW(Order(1, 42, Side::Buy, OrderType::Market, 1, Price{1}), std::invalid_argument);
}

TEST(Order, RejectsInvalidTransitions) {
    Order order{5, 42, Side::Buy, OrderType::Limit, 100, Price{10000}};

    EXPECT_THROW(order.execute(0), std::invalid_argument);
    EXPECT_THROW(order.execute(101), std::invalid_argument);
    order.execute(100);
    EXPECT_THROW(order.execute(1), std::logic_error);
    EXPECT_THROW(order.cancel(), std::logic_error);
}

}  // namespace
