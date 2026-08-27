#pragma once

#include <cstdint>
#include <optional>
#include <random>
#include <variant>

#include "aegisx/order_book.hpp"

namespace aegisx {

struct SubmitEvent {
    Order order;
};

struct CancelEvent {
    OrderId order_id;
};

struct ModifyEvent {
    OrderId order_id;
    Quantity new_quantity;
    std::optional<Price> new_price;
};

using SimulationEvent = std::variant<SubmitEvent, CancelEvent, ModifyEvent>;

class OrderGenerator {
public:
    explicit OrderGenerator(std::uint64_t seed, InstrumentId instrument_id = 1);

    [[nodiscard]] SimulationEvent next(const OrderBook& book);

private:
    [[nodiscard]] SimulationEvent make_submit();
    [[nodiscard]] SimulationEvent make_cancel(const std::vector<const Order*>& active_orders);
    [[nodiscard]] SimulationEvent make_modify(const std::vector<const Order*>& active_orders);

    std::mt19937_64 random_engine_;
    InstrumentId instrument_id_;
    OrderId next_order_id_{1};
};

}  // namespace aegisx
