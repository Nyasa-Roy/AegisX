#pragma once

#include <vector>

#include "aegisx/order.hpp"
#include "aegisx/trade.hpp"

namespace aegisx {

class ReferenceEngine {
public:
    explicit ReferenceEngine(InstrumentId instrument_id);

    [[nodiscard]] InstrumentId instrument_id() const noexcept;
    [[nodiscard]] const std::vector<Order>& orders() const noexcept;

    std::vector<Trade> submit(Order order);
    void cancel(OrderId order_id);
    void modify(OrderId order_id, Quantity new_quantity,
                std::optional<Price> new_price = std::nullopt);

private:
    [[nodiscard]] std::size_t find_order(OrderId order_id) const;
    [[nodiscard]] std::optional<std::size_t> find_best(Side incoming_side) const;

    InstrumentId instrument_id_;
    std::vector<Order> orders_;
};

}  // namespace aegisx
