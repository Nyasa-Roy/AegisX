#include "aegisx/price_level.hpp"

#include <stdexcept>
#include <utility>

namespace aegisx {

PriceLevel::PriceLevel(Price price) : price_(price) {
    if (price_ <= 0) {
        throw std::invalid_argument("price level price must be positive");
    }
}

void PriceLevel::add_order(Order order) {
    if (order.type() != OrderType::Limit || !order.price().has_value()) {
        throw std::invalid_argument("only priced limit orders can rest at a price level");
    }
    if (order.price().value() != price_) {
        throw std::invalid_argument("order price must match the price level");
    }
    orders_.push_back(std::move(order));
}

Price PriceLevel::price() const noexcept { return price_; }
bool PriceLevel::empty() const noexcept { return orders_.empty(); }
std::size_t PriceLevel::size() const noexcept { return orders_.size(); }

Quantity PriceLevel::total_quantity() const noexcept {
    Quantity total = 0;
    for (const Order& order : orders_) {
        total += order.remaining_quantity();
    }
    return total;
}

const Order& PriceLevel::front() const {
    if (orders_.empty()) {
        throw std::out_of_range("cannot access the front of an empty price level");
    }
    return orders_.front();
}

Order& PriceLevel::front_mutable() {
    if (orders_.empty()) {
        throw std::out_of_range("cannot access the front of an empty price level");
    }
    return orders_.front();
}

void PriceLevel::pop_front() {
    if (orders_.empty()) {
        throw std::out_of_range("cannot remove the front of an empty price level");
    }
    orders_.pop_front();
}

const std::deque<Order>& PriceLevel::orders() const noexcept { return orders_; }

}  // namespace aegisx
