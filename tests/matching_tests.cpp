#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

#include "aegisx/matching_engine.hpp"

namespace {

using namespace aegisx;

Order buy(OrderId id, Quantity quantity, Price price) {
    return Order{id, 7, Side::Buy, OrderType::Limit, quantity, price};
}

Order sell(OrderId id, Quantity quantity, Price price) {
    return Order{id, 7, Side::Sell, OrderType::Limit, quantity, price};
}

TEST(MatchingEngine, ExactFillRemovesRestingOrder) {
    MatchingEngine engine{7};
    engine.submit(sell(1, 100, 100));

    const std::vector<Trade> trades = engine.submit(buy(2, 100, 100));

    ASSERT_EQ(trades.size(), 1U);
    EXPECT_EQ(trades[0].maker_order_id, 1U);
    EXPECT_EQ(trades[0].taker_order_id, 2U);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 100U);
    EXPECT_TRUE(engine.book().empty());
}

TEST(MatchingEngine, PartialFillLeavesRestingQuantity) {
    MatchingEngine engine{7};
    engine.submit(sell(1, 100, 100));

    const std::vector<Trade> trades = engine.submit(buy(2, 40, 100));

    ASSERT_EQ(trades.size(), 1U);
    EXPECT_EQ(trades[0].quantity, 40U);
    ASSERT_NE(engine.book().best_ask(), nullptr);
    EXPECT_EQ(engine.book().best_ask()->front().id(), 1U);
    EXPECT_EQ(engine.book().best_ask()->front().remaining_quantity(), 60U);
}

TEST(MatchingEngine, MatchesAcrossMultiplePriceLevels) {
    MatchingEngine engine{7};
    engine.submit(sell(1, 50, 100));
    engine.submit(sell(2, 50, 101));
    engine.submit(sell(3, 50, 102));

    const std::vector<Trade> trades = engine.submit(buy(4, 120, 102));

    ASSERT_EQ(trades.size(), 3U);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 50U);
    EXPECT_EQ(trades[1].price, 101);
    EXPECT_EQ(trades[1].quantity, 50U);
    EXPECT_EQ(trades[2].price, 102);
    EXPECT_EQ(trades[2].quantity, 20U);
    ASSERT_NE(engine.book().best_ask(), nullptr);
    EXPECT_EQ(engine.book().best_ask()->front().id(), 3U);
    EXPECT_EQ(engine.book().best_ask()->front().remaining_quantity(), 30U);
}

TEST(MatchingEngine, PreservesFIFOAtSamePrice) {
    MatchingEngine engine{7};
    engine.submit(sell(1, 40, 100));
    engine.submit(sell(2, 40, 100));

    const std::vector<Trade> trades = engine.submit(buy(3, 50, 100));

    ASSERT_EQ(trades.size(), 2U);
    EXPECT_EQ(trades[0].maker_order_id, 1U);
    EXPECT_EQ(trades[0].quantity, 40U);
    EXPECT_EQ(trades[1].maker_order_id, 2U);
    EXPECT_EQ(trades[1].quantity, 10U);
    ASSERT_NE(engine.book().best_ask(), nullptr);
    EXPECT_EQ(engine.book().best_ask()->front().id(), 2U);
    EXPECT_EQ(engine.book().best_ask()->front().remaining_quantity(), 30U);
}

TEST(MatchingEngine, NonCrossingOrderRestsOnItsSide) {
    MatchingEngine engine{7};
    engine.submit(sell(1, 100, 105));

    const std::vector<Trade> trades = engine.submit(buy(2, 100, 104));

    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(engine.book().order_count(), 2U);
    ASSERT_NE(engine.book().best_bid(), nullptr);
    EXPECT_EQ(engine.book().best_bid()->price(), 104);
    ASSERT_NE(engine.book().best_ask(), nullptr);
    EXPECT_EQ(engine.book().best_ask()->price(), 105);
}

TEST(MatchingEngine, MarketBuyConsumesBestAvailableLiquidity) {
    MatchingEngine engine{7};
    engine.submit(sell(1, 50, 100));
    engine.submit(sell(2, 75, 101));

    const std::vector<Trade> trades =
        engine.submit(Order{3, 7, Side::Buy, OrderType::Market, 100});

    ASSERT_EQ(trades.size(), 2U);
    EXPECT_EQ(trades[0].maker_order_id, 1U);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 50U);
    EXPECT_EQ(trades[1].maker_order_id, 2U);
    EXPECT_EQ(trades[1].price, 101);
    EXPECT_EQ(trades[1].quantity, 50U);
    ASSERT_NE(engine.book().best_ask(), nullptr);
    EXPECT_EQ(engine.book().best_ask()->front().id(), 2U);
    EXPECT_EQ(engine.book().best_ask()->front().remaining_quantity(), 25U);
}

TEST(MatchingEngine, MarketOrderRemainderIsDiscarded) {
    MatchingEngine engine{7};
    engine.submit(sell(1, 40, 100));

    const std::vector<Trade> trades =
        engine.submit(Order{2, 7, Side::Buy, OrderType::Market, 100});

    ASSERT_EQ(trades.size(), 1U);
    EXPECT_EQ(trades[0].quantity, 40U);
    EXPECT_TRUE(engine.book().empty());
    EXPECT_EQ(engine.book().best_bid(), nullptr);
}

TEST(MatchingEngine, EmptyBookMarketOrderProducesNoTrade) {
    MatchingEngine engine{7};

    const std::vector<Trade> trades =
        engine.submit(Order{1, 7, Side::Buy, OrderType::Market, 100});

    EXPECT_TRUE(trades.empty());
    EXPECT_TRUE(engine.book().empty());
}

TEST(MatchingEngine, MarketSellConsumesHighestBidFirst) {
    MatchingEngine engine{7};
    engine.submit(buy(1, 50, 100));
    engine.submit(buy(2, 75, 101));

    const std::vector<Trade> trades =
        engine.submit(Order{3, 7, Side::Sell, OrderType::Market, 100});

    ASSERT_EQ(trades.size(), 2U);
    EXPECT_EQ(trades[0].maker_order_id, 2U);
    EXPECT_EQ(trades[0].price, 101);
    EXPECT_EQ(trades[0].quantity, 75U);
    EXPECT_EQ(trades[1].maker_order_id, 1U);
    EXPECT_EQ(trades[1].price, 100);
    EXPECT_EQ(trades[1].quantity, 25U);
    EXPECT_TRUE(engine.book().best_ask() == nullptr);
    ASSERT_NE(engine.book().best_bid(), nullptr);
    EXPECT_EQ(engine.book().best_bid()->front().id(), 1U);
    EXPECT_EQ(engine.book().best_bid()->front().remaining_quantity(), 25U);
}

TEST(MatchingEngine, RejectsUnsupportedOrInvalidOrders) {
    MatchingEngine engine{7};

    EXPECT_THROW(engine.submit(Order{2, 8, Side::Buy, OrderType::Limit, 10, Price{100}}), std::invalid_argument);

    engine.submit(sell(3, 10, 100));
    EXPECT_THROW(engine.submit(sell(3, 10, 101)), std::invalid_argument);
}

}  // namespace
