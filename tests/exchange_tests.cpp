#include <stdexcept>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "aegisx/exchange.hpp"

namespace {

using namespace aegisx;

Order limit(OrderId id, InstrumentId instrument_id, Side side, Quantity quantity, Price price) {
    return Order{id, instrument_id, side, OrderType::Limit, quantity, price};
}

TEST(Exchange, MaintainsIndependentInstrumentBooks) {
    Exchange exchange;
    exchange.add_instrument(1);
    exchange.add_instrument(2);

    exchange.submit_events(limit(10, 1, Side::Sell, 100, 100));
    const std::vector<Event> events = exchange.submit_events(
        limit(11, 2, Side::Buy, 100, 100));

    ASSERT_EQ(events.size(), 2U);
    EXPECT_TRUE(std::holds_alternative<OrderAccepted>(events[0]));
    EXPECT_TRUE(std::holds_alternative<OrderRested>(events[1]));
    ASSERT_NE(exchange.instrument_engine(1), nullptr);
    ASSERT_NE(exchange.instrument_engine(2), nullptr);
    EXPECT_EQ(exchange.instrument_engine(1)->book().best_ask()->front().id(), 10U);
    EXPECT_EQ(exchange.instrument_engine(2)->book().best_bid()->front().id(), 11U);
}

TEST(Exchange, RoutesMatchingToTheCorrectInstrument) {
    Exchange exchange;
    exchange.add_instrument(1);
    exchange.add_instrument(2);
    exchange.submit_events(limit(10, 1, Side::Sell, 100, 100));

    const std::vector<Event> events = exchange.submit_events(
        limit(11, 1, Side::Buy, 100, 100));

    ASSERT_EQ(events.size(), 2U);
    EXPECT_TRUE(std::holds_alternative<TradeExecuted>(events[1]));
    EXPECT_TRUE(exchange.instrument_engine(1)->book().empty());
    EXPECT_TRUE(exchange.instrument_engine(2)->book().empty());
}

TEST(Exchange, SupportsRoutedCancellationAndModification) {
    Exchange exchange;
    exchange.add_instrument(1);
    exchange.submit_events(limit(10, 1, Side::Buy, 100, 100));

    const std::vector<Event> modified = exchange.modify(1, 10, 50);
    EXPECT_TRUE(std::holds_alternative<OrderModified>(modified[0]));
    const std::vector<Event> cancelled = exchange.cancel(1, 10);
    EXPECT_TRUE(std::holds_alternative<OrderCancelled>(cancelled[0]));
    EXPECT_TRUE(exchange.instrument_engine(1)->book().empty());
}

TEST(Exchange, RejectsUnknownAndDuplicateInstruments) {
    Exchange exchange;
    EXPECT_THROW(exchange.add_instrument(0), std::invalid_argument);
    exchange.add_instrument(1);
    EXPECT_THROW(exchange.add_instrument(1), std::invalid_argument);
    EXPECT_THROW(exchange.submit_events(limit(10, 2, Side::Buy, 1, 100)), std::invalid_argument);
    EXPECT_THROW(exchange.cancel(2, 10), std::invalid_argument);
    EXPECT_THROW(exchange.modify(2, 10, 2), std::invalid_argument);
}

}  // namespace
