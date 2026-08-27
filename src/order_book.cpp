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
    const Side side = order.side();
    const Price price = order.price().value();
    if (side == Side::Buy) {
        auto [level, inserted] = bids_.try_emplace(price, price);
        (void)inserted;
        level->second.add_order(std::move(order));
    } else {
        auto [level, inserted] = asks_.try_emplace(price, price);
        (void)inserted;
        level->second.add_order(std::move(order));
    }
    order_ids_.insert(order_id);
    locations_.emplace(order_id, OrderLocation{side, price});
}

std::optional<Order> OrderBook::extract_order(OrderId order_id) {
    const auto location = locations_.find(order_id);
    if (location == locations_.end()) {
        return std::nullopt;
    }

    if (location->second.side == Side::Buy) {
        auto iterator = bids_.find(location->second.price);
        if (iterator != bids_.end()) {
            std::optional<Order> removed_order = iterator->second.remove_order(order_id);
            if (removed_order.has_value()) {
                order_ids_.erase(order_id);
                locations_.erase(location);
                if (iterator->second.empty()) {
                    bids_.erase(iterator);
                }
                return removed_order;
            }
        }
    } else {
        auto iterator = asks_.find(location->second.price);
        if (iterator != asks_.end()) {
            std::optional<Order> removed_order = iterator->second.remove_order(order_id);
            if (removed_order.has_value()) {
                order_ids_.erase(order_id);
                locations_.erase(location);
                if (iterator->second.empty()) {
                    asks_.erase(iterator);
                }
                return removed_order;
            }
        }
    }
    return std::nullopt;
}

std::optional<Order> OrderBook::cancel_order(OrderId order_id) {
    const auto location = locations_.find(order_id);
    if (location == locations_.end()) {
        return std::nullopt;
    }

    if (location->second.side == Side::Buy) {
        auto iterator = bids_.find(location->second.price);
        if (iterator != bids_.end()) {
            std::optional<Order> cancelled_order = iterator->second.cancel_order(order_id);
            if (cancelled_order.has_value()) {
                order_ids_.erase(order_id);
                locations_.erase(location);
                if (iterator->second.empty()) {
                    bids_.erase(iterator);
                }
                return cancelled_order;
            }
        }
    } else {
        auto iterator = asks_.find(location->second.price);
        if (iterator != asks_.end()) {
            std::optional<Order> cancelled_order = iterator->second.cancel_order(order_id);
            if (cancelled_order.has_value()) {
                order_ids_.erase(order_id);
                locations_.erase(location);
                if (iterator->second.empty()) {
                    asks_.erase(iterator);
                }
                return cancelled_order;
            }
        }
    }
    return std::nullopt;
}

InstrumentId OrderBook::instrument_id() const noexcept { return instrument_id_; }
bool OrderBook::empty() const noexcept { return order_ids_.empty(); }
std::size_t OrderBook::order_count() const noexcept { return order_ids_.size(); }
std::size_t OrderBook::bid_level_count() const noexcept { return bids_.size(); }
std::size_t OrderBook::ask_level_count() const noexcept { return asks_.size(); }
bool OrderBook::contains_order(OrderId order_id) const noexcept {
    return order_ids_.contains(order_id);
}

std::vector<const Order*> OrderBook::all_orders() const {
    std::vector<const Order*> orders;
    orders.reserve(order_ids_.size());
    for (const auto& [price, level] : bids_) {
        (void)price;
        for (const Order& order : level.orders()) {
            orders.push_back(&order);
        }
    }
    for (const auto& [price, level] : asks_) {
        (void)price;
        for (const Order& order : level.orders()) {
            orders.push_back(&order);
        }
    }
    return orders;
}

const Order* OrderBook::find_order(OrderId order_id) const noexcept {
    const auto location = locations_.find(order_id);
    if (location == locations_.end()) {
        return nullptr;
    }
    if (location->second.side == Side::Buy) {
        const auto iterator = bids_.find(location->second.price);
        if (iterator == bids_.end()) {
            return nullptr;
        }
        const auto& level = iterator->second;
        for (const Order& order : level.orders()) {
            if (order.id() == order_id) {
                return &order;
            }
        }
    } else {
        const auto iterator = asks_.find(location->second.price);
        if (iterator == asks_.end()) {
            return nullptr;
        }
        const auto& level = iterator->second;
        for (const Order& order : level.orders()) {
            if (order.id() == order_id) {
                return &order;
            }
        }
    }
    return nullptr;
}

Order* OrderBook::find_order_mutable(OrderId order_id) noexcept {
    const auto location = locations_.find(order_id);
    if (location == locations_.end()) {
        return nullptr;
    }
    if (location->second.side == Side::Buy) {
        auto iterator = bids_.find(location->second.price);
        if (iterator == bids_.end()) {
            return nullptr;
        }
        auto& level = iterator->second;
        for (Order& order : level.orders()) {
            if (order.id() == order_id) {
                return &order;
            }
        }
    } else {
        auto iterator = asks_.find(location->second.price);
        if (iterator == asks_.end()) {
            return nullptr;
        }
        auto& level = iterator->second;
        for (Order& order : level.orders()) {
            if (order.id() == order_id) {
                return &order;
            }
        }
    }
    return nullptr;
}

const PriceLevel* OrderBook::best_bid() const noexcept {
    return bids_.empty() ? nullptr : &bids_.begin()->second;
}

const PriceLevel* OrderBook::best_ask() const noexcept {
    return asks_.empty() ? nullptr : &asks_.begin()->second;
}

Order* OrderBook::best_bid_order() noexcept {
    return bids_.empty() ? nullptr : &bids_.begin()->second.front_mutable();
}

Order* OrderBook::best_ask_order() noexcept {
    return asks_.empty() ? nullptr : &asks_.begin()->second.front_mutable();
}

void OrderBook::remove_filled_best(Side side) {
    if (side == Side::Buy) {
        if (bids_.empty() || bids_.begin()->second.empty()) {
            throw std::logic_error("cannot remove an order from an empty bid book");
        }
        PriceLevel& level = bids_.begin()->second;
        Order& order = level.front_mutable();
        if (order.status() != OrderStatus::Filled) {
            throw std::logic_error("only filled orders can be removed from the book");
        }
        order_ids_.erase(order.id());
        locations_.erase(order.id());
        level.pop_front();
        if (level.empty()) {
            bids_.erase(bids_.begin());
        }
    } else {
        if (asks_.empty() || asks_.begin()->second.empty()) {
            throw std::logic_error("cannot remove an order from an empty ask book");
        }
        PriceLevel& level = asks_.begin()->second;
        Order& order = level.front_mutable();
        if (order.status() != OrderStatus::Filled) {
            throw std::logic_error("only filled orders can be removed from the book");
        }
        order_ids_.erase(order.id());
        locations_.erase(order.id());
        level.pop_front();
        if (level.empty()) {
            asks_.erase(asks_.begin());
        }
    }
}

}  // namespace aegisx
