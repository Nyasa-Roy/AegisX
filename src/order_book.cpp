#include "aegisx/order_book.hpp"

#include <stdexcept>
#include <utility>

namespace aegisx {

OrderBook::OrderBook(InstrumentId instrument_id) : instrument_id_(instrument_id) {
    if (instrument_id_ == 0) {
        throw std::invalid_argument("order book instrument id must be non-zero");
    }
}

void OrderBook::add_order(Order order) {
    if (order.instrument_id() != instrument_id_) {
        throw std::invalid_argument("order instrument does not match the order book");
    }
    if (order.type() != OrderType::Limit || !order.price().has_value()) {
        throw std::invalid_argument("only limit orders can rest in an order book");
    }
    if (order_ids_.contains(order.id())) {
        throw std::invalid_argument("duplicate order id in order book");
    }

    const OrderId order_id = order.id();
    const Price price = order.price().value();
    if (order.side() == Side::Buy) {
        auto [level, inserted] = bids_.try_emplace(price, price);
        (void)inserted;
        level->second.add_order(std::move(order));
    } else {
        auto [level, inserted] = asks_.try_emplace(price, price);
        (void)inserted;
        level->second.add_order(std::move(order));
    }
    order_ids_.insert(order_id);
}

InstrumentId OrderBook::instrument_id() const noexcept { return instrument_id_; }
bool OrderBook::empty() const noexcept { return order_ids_.empty(); }
std::size_t OrderBook::order_count() const noexcept { return order_ids_.size(); }
std::size_t OrderBook::bid_level_count() const noexcept { return bids_.size(); }
std::size_t OrderBook::ask_level_count() const noexcept { return asks_.size(); }
bool OrderBook::contains_order(OrderId order_id) const noexcept {
    return order_ids_.contains(order_id);
}

const PriceLevel* OrderBook::best_bid() const noexcept {
    return bids_.empty() ? nullptr : &bids_.begin()->second;
}

const PriceLevel* OrderBook::best_ask() const noexcept {
    return asks_.empty() ? nullptr : &asks_.begin()->second;
}

}  // namespace aegisx
