#pragma once

#include "aegisx/types.hpp"

namespace aegisx {

struct Trade {
    InstrumentId instrument_id;
    OrderId maker_order_id;
    OrderId taker_order_id;
    Price price;
    Quantity quantity;

    Trade(InstrumentId instrument_id_value,
          OrderId maker_order_id_value,
          OrderId taker_order_id_value,
          Price price_value,
          Quantity quantity_value);
};

}  // namespace aegisx
