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

using Event = std::variant<OrderAccepted, TradeExecuted, OrderRested, OrderCancelled>;

}  // namespace aegisx
