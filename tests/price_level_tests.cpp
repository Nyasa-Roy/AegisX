#include <stdexcept>

#include <gtest/gtest.h>

#include "aegisx/price_level.hpp"

namespace {

using namespace aegisx;

Order make_order(OrderId id, Quantity quantity, Price price = 100) {
    return Order{id, 7, Side::Buy, OrderType::Limit, quantity, price};
}

TEST(PriceLevel, PreservesFIFOOrderAtOnePrice) {
    PriceLevel level{100};
    level.add_order(make_order(1, 50));
    level.add_order(make_order(2, 75));

    ASSERT_EQ(level.size(), 2U);
    EXPECT_EQ(level.front().id(), 1U);
    EXPECT_EQ(level.orders().at(1).id(), 2U);
    EXPECT_EQ(level.total_quantity(), 125U);
}

TEST(PriceLevel, RejectsMismatchedOrMarketOrders) {
    PriceLevel level{100};

    EXPECT_THROW(level.add_order(make_order(1, 10, 101)), std::invalid_argument);
    EXPECT_THROW(level.add_order(Order{2, 7, Side::Buy, OrderType::Market, 10}), std::invalid_argument);
}

TEST(PriceLevel, RejectsInvalidPriceAndEmptyFront) {
    EXPECT_THROW(PriceLevel{0}, std::invalid_argument);

    PriceLevel level{100};
    EXPECT_THROW(static_cast<void>(level.front()), std::out_of_range);
}

}  // namespace
