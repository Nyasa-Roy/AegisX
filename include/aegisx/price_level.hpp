#pragma once

#include <cstddef>
#include <deque>
#include <optional>

#include "aegisx/order.hpp"

namespace aegisx {

class PriceLevel {
public:
    explicit PriceLevel(Price price);

    void add_order(Order order);
    [[nodiscard]] std::optional<Order> cancel_order(OrderId order_id);

    [[nodiscard]] Price price() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] Quantity total_quantity() const noexcept;
    [[nodiscard]] const Order& front() const;
    [[nodiscard]] Order& front_mutable();
    void pop_front();
    [[nodiscard]] const std::deque<Order>& orders() const noexcept;

private:
    Price price_;
    std::deque<Order> orders_;
};

}  // namespace aegisx
