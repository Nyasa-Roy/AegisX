#include "aegisx/reference_engine.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace aegisx {

ReferenceEngine::ReferenceEngine(InstrumentId instrument_id) : instrument_id_(instrument_id) {
    if (instrument_id_ == 0) {
        throw std::invalid_argument("reference engine instrument id must be non-zero");
    }
}

InstrumentId ReferenceEngine::instrument_id() const noexcept { return instrument_id_; }
const std::vector<Order>& ReferenceEngine::orders() const noexcept { return orders_; }

std::size_t ReferenceEngine::find_order(OrderId order_id) const {
    for (std::size_t index = 0; index < orders_.size(); ++index) {
        if (orders_[index].id() == order_id) {
            return index;
        }
    }
    throw std::invalid_argument("order is not active in reference engine");
}

std::optional<std::size_t> ReferenceEngine::find_best(Side incoming_side) const {
    std::optional<std::size_t> best_index;
    for (std::size_t index = 0; index < orders_.size(); ++index) {
        const Order& candidate = orders_[index];
        if (candidate.side() == incoming_side) {
            continue;
        }
        if (!best_index.has_value()) {
            best_index = index;
            continue;
        }
        const Price candidate_price = candidate.price().value();
        const Price best_price = orders_[best_index.value()].price().value();
        if ((incoming_side == Side::Buy && candidate_price < best_price) ||
            (incoming_side == Side::Sell && candidate_price > best_price)) {
            best_index = index;
        }
    }
    return best_index;
}

std::vector<Trade> ReferenceEngine::submit(Order order) {
    if (order.instrument_id() != instrument_id_) {
        throw std::invalid_argument("order instrument does not match reference engine");
    }
    for (const Order& existing : orders_) {
        if (existing.id() == order.id()) {
            throw std::invalid_argument("duplicate order id in reference engine");
        }
    }

    std::vector<Trade> trades;
    while (order.is_active() && order.remaining_quantity() > 0) {
        const std::optional<std::size_t> best = find_best(order.side());
        if (!best.has_value()) {
            break;
        }
        const Order& candidate = orders_[best.value()];
        const Price candidate_price = candidate.price().value();
        const bool crosses = order.type() == OrderType::Market ||
            (order.side() == Side::Buy ? order.price().value() >= candidate_price
                                       : order.price().value() <= candidate_price);
        if (!crosses) {
            break;
        }

        Order& resting = orders_[best.value()];
        const Quantity quantity = std::min(order.remaining_quantity(), resting.remaining_quantity());
        const OrderId maker_id = resting.id();
        resting.execute(quantity);
        order.execute(quantity);
        trades.emplace_back(instrument_id_, maker_id, order.id(), candidate_price, quantity);
        if (resting.status() == OrderStatus::Filled) {
            orders_.erase(orders_.begin() + static_cast<std::ptrdiff_t>(best.value()));
        }
    }
    if (order.type() == OrderType::Limit && order.is_active() && order.remaining_quantity() > 0) {
        orders_.push_back(std::move(order));
    }
    return trades;
}

void ReferenceEngine::cancel(OrderId order_id) {
    const std::size_t index = find_order(order_id);
    orders_[index].cancel();
    orders_.erase(orders_.begin() + static_cast<std::ptrdiff_t>(index));
}

void ReferenceEngine::modify(OrderId order_id, Quantity new_quantity,
                              std::optional<Price> new_price) {
    const std::size_t index = find_order(order_id);
    const Order& current = orders_[index];
    const bool price_changed = new_price.has_value() && new_price != current.price();
    const bool priority_reset = price_changed || new_quantity > current.original_quantity();
    if (priority_reset) {
        Order modified = current;
        modified.modify(new_quantity, new_price);
        orders_.erase(orders_.begin() + static_cast<std::ptrdiff_t>(index));
        orders_.push_back(std::move(modified));
    } else {
        orders_[index].modify(new_quantity, new_price);
    }
}

}  // namespace aegisx
