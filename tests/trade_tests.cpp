#include <stdexcept>

#include <gtest/gtest.h>

#include "aegisx/trade.hpp"

namespace {

using namespace aegisx;

TEST(Trade, StoresIntegerPricedExecution) {
    const Trade trade{42, 10, 20, Price{15025}, 7};

    EXPECT_EQ(trade.instrument_id, 42U);
    EXPECT_EQ(trade.maker_order_id, 10U);
    EXPECT_EQ(trade.taker_order_id, 20U);
    EXPECT_EQ(trade.price, 15025);
    EXPECT_EQ(trade.quantity, 7U);
}

TEST(Trade, RejectsInvalidValues) {
    EXPECT_THROW(Trade(0, 10, 20, Price{100}, 1), std::invalid_argument);
    EXPECT_THROW(Trade(42, 0, 20, Price{100}, 1), std::invalid_argument);
    EXPECT_THROW(Trade(42, 10, 0, Price{100}, 1), std::invalid_argument);
    EXPECT_THROW(Trade(42, 10, 20, Price{0}, 1), std::invalid_argument);
    EXPECT_THROW(Trade(42, 10, 20, Price{100}, 0), std::invalid_argument);
}

}  // namespace
