#include <stdexcept>

#include <gtest/gtest.h>

#include "aegisx/order_book.hpp"

namespace {

using namespace aegisx;

Order buy(OrderId id, Price price, Quantity quantity = 100) {
    return Order{id, 7, Side::Buy, OrderType::Limit, quantity, price};
}

Order sell(OrderId id, Price price, Quantity quantity = 100) {
    return Order{id, 7, Side::Sell, OrderType::Limit, quantity, price};
}

TEST(OrderBook, SelectsHighestPriceAsBestBid) {
    OrderBook book{7};
    book.add_order(buy(1, 100));
    book.add_order(buy(2, 101));

    ASSERT_NE(book.best_bid(), nullptr);
    EXPECT_EQ(book.best_bid()->price(), 101);
    EXPECT_EQ(book.best_bid()->front().id(), 2U);
    EXPECT_EQ(book.best_ask(), nullptr);
}

TEST(OrderBook, SelectsLowestPriceAsBestAsk) {
    OrderBook book{7};
    book.add_order(sell(1, 101));
    book.add_order(sell(2, 100));

    ASSERT_NE(book.best_ask(), nullptr);
    EXPECT_EQ(book.best_ask()->price(), 100);
    EXPECT_EQ(book.best_ask()->front().id(), 2U);
    EXPECT_EQ(book.best_bid(), nullptr);
}

TEST(OrderBook, PreservesFIFOWithinSamePrice) {
    OrderBook book{7};
    book.add_order(buy(1, 100, 10));
    book.add_order(buy(2, 100, 20));

    ASSERT_NE(book.best_bid(), nullptr);
    EXPECT_EQ(book.best_bid()->front().id(), 1U);
    EXPECT_EQ(book.best_bid()->orders().at(1).id(), 2U);
}

TEST(OrderBook, TracksLevelsOrdersAndMembership) {
    OrderBook book{7};
    book.add_order(buy(1, 100));
    book.add_order(buy(2, 101));
    book.add_order(sell(3, 102));

    EXPECT_FALSE(book.empty());
    EXPECT_EQ(book.order_count(), 3U);
    EXPECT_EQ(book.bid_level_count(), 2U);
    EXPECT_EQ(book.ask_level_count(), 1U);
    EXPECT_TRUE(book.contains_order(1));
    EXPECT_FALSE(book.contains_order(99));
}

TEST(OrderBook, RejectsInvalidRestingOrders) {
    EXPECT_THROW(OrderBook{0}, std::invalid_argument);

    OrderBook book{7};
    book.add_order(buy(1, 100));
    EXPECT_THROW(book.add_order(buy(1, 101)), std::invalid_argument);
    EXPECT_THROW(book.add_order(Order{2, 8, Side::Buy, OrderType::Limit, 10, Price{100}}), std::invalid_argument);
    EXPECT_THROW(book.add_order(Order{3, 7, Side::Buy, OrderType::Market, 10}), std::invalid_argument);
}

}  // namespace
