#include "aegisx/order.hpp"

#include <stdexcept>

namespace aegisx {

Order::Order(OrderId id,
             InstrumentId instrument_id,
             Side side,
             OrderType type,
             Quantity quantity,
             std::optional<Price> price)
    : id_(id),
      instrument_id_(instrument_id),
      side_(side),
      type_(type),
      price_(price),
      original_quantity_(quantity),
      remaining_quantity_(quantity) {
    if (id_ == 0) {
        throw std::invalid_argument("order id must be non-zero");
    }
    if (instrument_id_ == 0) {
        throw std::invalid_argument("instrument id must be non-zero");
    }
    if (quantity == 0) {
        throw std::invalid_argument("order quantity must be non-zero");
    }
    if (type_ == OrderType::Limit) {
        if (!price_.has_value() || price_.value() <= 0) {
            throw std::invalid_argument("limit orders require a positive price");
        }
    } else if (price_.has_value()) {
        throw std::invalid_argument("market orders must not have a price");
    }
}

OrderId Order::id() const noexcept { return id_; }
InstrumentId Order::instrument_id() const noexcept { return instrument_id_; }
Side Order::side() const noexcept { return side_; }
OrderType Order::type() const noexcept { return type_; }
std::optional<Price> Order::price() const noexcept { return price_; }
Quantity Order::original_quantity() const noexcept { return original_quantity_; }
Quantity Order::executed_quantity() const noexcept { return executed_quantity_; }
Quantity Order::remaining_quantity() const noexcept { return remaining_quantity_; }
OrderStatus Order::status() const noexcept { return status_; }
bool Order::is_active() const noexcept {
    return status_ == OrderStatus::Active || status_ == OrderStatus::PartiallyFilled;
}

void Order::execute(Quantity quantity) {
    if (!is_active()) {
        throw std::logic_error("only active orders can execute");
    }
    if (quantity == 0 || quantity > remaining_quantity_) {
        throw std::invalid_argument("execution quantity must be positive and within remaining quantity");
    }

    executed_quantity_ += quantity;
    remaining_quantity_ -= quantity;
    status_ = remaining_quantity_ == 0 ? OrderStatus::Filled : OrderStatus::PartiallyFilled;
}

void Order::cancel() {
    if (!is_active()) {
        throw std::logic_error("only active orders can be cancelled");
    }
    status_ = OrderStatus::Cancelled;
}

}  // namespace aegisx
