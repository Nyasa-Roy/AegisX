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

std::vector<Event> MatchingEngine::cancel(OrderId order_id) {
    std::optional<Order> cancelled_order = book_.cancel_order(order_id);
    if (!cancelled_order.has_value()) {
        throw std::invalid_argument("order is not active in the matching engine");
    }

    std::vector<Event> events;
    events.emplace_back(OrderCancelled{
        cancelled_order->id(), cancelled_order->instrument_id(), cancelled_order->side(),
        cancelled_order->remaining_quantity()});
    return events;
}

std::vector<Event> MatchingEngine::modify(OrderId order_id, Quantity new_quantity,
                                           std::optional<Price> new_price) {
    const Order* current_order = book_.find_order(order_id);
    if (current_order == nullptr) {
        throw std::invalid_argument("order is not active in the matching engine");
    }

    const Quantity old_quantity = current_order->original_quantity();
    const std::optional<Price> old_price = current_order->price();
    const bool price_changed = new_price.has_value() && new_price != old_price;
    const bool priority_reset = price_changed || new_quantity > old_quantity;

    if (priority_reset) {
        Order validated_order = *current_order;
        validated_order.modify(new_quantity, new_price);
        std::optional<Order> extracted_order = book_.extract_order(order_id);
        if (!extracted_order.has_value()) {
            throw std::logic_error("order disappeared during modification");
        }
        extracted_order->modify(new_quantity, new_price);
        const Order& modified_order = extracted_order.value();
        const Price modified_price = modified_order.price().value();
        const Quantity remaining_quantity = modified_order.remaining_quantity();
        const Side side = modified_order.side();
        book_.add_order(std::move(extracted_order.value()));
        return {Event{OrderModified{order_id, instrument_id(), side, modified_price,
                                    new_quantity, remaining_quantity, true}}};
    }

    Order* mutable_order = book_.find_order_mutable(order_id);
    if (mutable_order == nullptr) {
        throw std::logic_error("order disappeared during modification");
    }
    mutable_order->modify(new_quantity, new_price);
    return {Event{OrderModified{order_id, instrument_id(), mutable_order->side(),
                                mutable_order->price().value(), mutable_order->original_quantity(),
                                mutable_order->remaining_quantity(), false}}};
}

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
