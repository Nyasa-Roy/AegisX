#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <unordered_set>

#include "aegisx/price_level.hpp"

namespace aegisx {

class OrderBook {
public:
    explicit OrderBook(InstrumentId instrument_id);

    void add_order(Order order);
    [[nodiscard]] std::optional<Order> cancel_order(OrderId order_id);

    [[nodiscard]] InstrumentId instrument_id() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t order_count() const noexcept;
    [[nodiscard]] std::size_t bid_level_count() const noexcept;
    [[nodiscard]] std::size_t ask_level_count() const noexcept;
    [[nodiscard]] bool contains_order(OrderId order_id) const noexcept;
    [[nodiscard]] const PriceLevel* best_bid() const noexcept;
    [[nodiscard]] const PriceLevel* best_ask() const noexcept;
    [[nodiscard]] Order* best_bid_order() noexcept;
    [[nodiscard]] Order* best_ask_order() noexcept;
    void remove_filled_best(Side side);

private:
    using BidLevels = std::map<Price, PriceLevel, std::greater<Price>>;
    using AskLevels = std::map<Price, PriceLevel>;

    InstrumentId instrument_id_;
    BidLevels bids_;
    AskLevels asks_;
    std::unordered_set<OrderId> order_ids_;
};

}  // namespace aegisx
