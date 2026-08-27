#pragma once

#include <optional>
#include <variant>

#include "aegisx/trade.hpp"

namespace aegisx {

struct OrderAccepted {
    OrderId order_id;
    InstrumentId instrument_id;
    Side side;
    OrderType type;
    Quantity quantity;
};

struct TradeExecuted {
    Trade trade;
};

struct OrderRested {
    OrderId order_id;
    InstrumentId instrument_id;
    Side side;
    Price price;
    Quantity remaining_quantity;
};

struct OrderCancelled {
    OrderId order_id;
    InstrumentId instrument_id;
    Side side;
    Quantity remaining_quantity;
};

struct OrderModified {
    OrderId order_id;
    InstrumentId instrument_id;
    Side side;
    Price price;
    Quantity original_quantity;
    Quantity remaining_quantity;
    bool priority_reset;
};

using Event = std::variant<OrderAccepted, TradeExecuted, OrderRested, OrderCancelled, OrderModified>;

}  // namespace aegisx
