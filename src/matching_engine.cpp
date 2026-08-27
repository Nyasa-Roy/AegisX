#include "aegisx/matching_engine.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace aegisx {

MatchingEngine::MatchingEngine(InstrumentId instrument_id) : book_(instrument_id) {}

InstrumentId MatchingEngine::instrument_id() const noexcept {
    return book_.instrument_id();
}

const OrderBook& MatchingEngine::book() const noexcept { return book_; }

std::vector<Trade> MatchingEngine::submit(Order order) {
    const std::vector<Event> events = submit_events(std::move(order));
    std::vector<Trade> trades;
    for (const Event& event : events) {
        if (const auto* trade_event = std::get_if<TradeExecuted>(&event)) {
            trades.push_back(trade_event->trade);
        }
    }
    return trades;
}

std::vector<Event> MatchingEngine::submit_events(Order order) {
    if (order.instrument_id() != instrument_id()) {
        throw std::invalid_argument("order instrument does not match the matching engine");
    }
    if (order.type() == OrderType::Limit && !order.price().has_value()) {
        throw std::invalid_argument("limit orders require a price");
    }
    if (book_.contains_order(order.id())) {
        throw std::invalid_argument("duplicate order id in matching engine");
    }

    std::vector<Event> events;
    events.emplace_back(OrderAccepted{
        order.id(), order.instrument_id(), order.side(), order.type(), order.original_quantity()});

    while (order.is_active() && order.remaining_quantity() > 0) {
        Order* resting_order = order.side() == Side::Buy
            ? book_.best_ask_order()
            : book_.best_bid_order();
        if (resting_order == nullptr) {
            break;
        }

        const Price resting_price = resting_order->price().value();
        const bool crosses = order.type() == OrderType::Market
            || (order.side() == Side::Buy
                ? order.price().value() >= resting_price
                : order.price().value() <= resting_price);
        if (!crosses) {
            break;
        }

        const Quantity execution_quantity = std::min(
            order.remaining_quantity(), resting_order->remaining_quantity());
        const OrderId maker_order_id = resting_order->id();
        resting_order->execute(execution_quantity);
        order.execute(execution_quantity);
        events.emplace_back(TradeExecuted{Trade{
            instrument_id(), maker_order_id, order.id(), resting_price, execution_quantity}});

        if (resting_order->status() == OrderStatus::Filled) {
            book_.remove_filled_best(resting_order->side());
        }
    }

    if (order.type() == OrderType::Limit && order.is_active() && order.remaining_quantity() > 0) {
        const OrderId order_id = order.id();
        const Side side = order.side();
        const Price price = order.price().value();
        const Quantity remaining_quantity = order.remaining_quantity();
        book_.add_order(std::move(order));
        events.emplace_back(OrderRested{
            order_id, instrument_id(), side, price, remaining_quantity});
    }
    return events;
}

}  // namespace aegisx
