#include "aegisx/trade.hpp"

#include <stdexcept>

namespace aegisx {

Trade::Trade(InstrumentId instrument_id_value,
             OrderId maker_order_id_value,
             OrderId taker_order_id_value,
             Price price_value,
             Quantity quantity_value)
    : instrument_id(instrument_id_value),
      maker_order_id(maker_order_id_value),
      taker_order_id(taker_order_id_value),
      price(price_value),
      quantity(quantity_value) {
    if (instrument_id_value == 0 || maker_order_id_value == 0 || taker_order_id_value == 0) {
        throw std::invalid_argument("trade identifiers must be non-zero");
    }
    if (price_value <= 0) {
        throw std::invalid_argument("trade price must be positive");
    }
    if (quantity_value == 0) {
        throw std::invalid_argument("trade quantity must be non-zero");
    }
}

}  // namespace aegisx
