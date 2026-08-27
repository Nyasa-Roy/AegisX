#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "aegisx/price_level.hpp"

namespace aegisx {

class OrderBook {
public:
    explicit OrderBook(InstrumentId instrument_id);

    void add_order(Order order);
    [[nodiscard]] std::optional<Order> extract_order(OrderId order_id);
    [[nodiscard]] std::optional<Order> cancel_order(OrderId order_id);

    [[nodiscard]] InstrumentId instrument_id() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t order_count() const noexcept;
    [[nodiscard]] std::size_t bid_level_count() const noexcept;
    [[nodiscard]] std::size_t ask_level_count() const noexcept;
    [[nodiscard]] bool contains_order(OrderId order_id) const noexcept;
    [[nodiscard]] std::vector<const Order*> all_orders() const;
    [[nodiscard]] const Order* find_order(OrderId order_id) const noexcept;
    [[nodiscard]] Order* find_order_mutable(OrderId order_id) noexcept;
    [[nodiscard]] const PriceLevel* best_bid() const noexcept;
    [[nodiscard]] const PriceLevel* best_ask() const noexcept;
    [[nodiscard]] Order* best_bid_order() noexcept;
    [[nodiscard]] Order* best_ask_order() noexcept;
    void remove_filled_best(Side side);

private:
    using BidLevels = std::map<Price, PriceLevel, std::greater<Price>>;
    using AskLevels = std::map<Price, PriceLevel>;
    struct OrderLocation {
        Side side;
        Price price;
    };

    InstrumentId instrument_id_;
    BidLevels bids_;
    AskLevels asks_;
    std::unordered_set<OrderId> order_ids_;
    std::unordered_map<OrderId, OrderLocation> locations_;
};

}  // namespace aegisx
