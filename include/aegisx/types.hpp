#pragma once

#include <cstdint>

namespace aegisx {

using OrderId = std::uint64_t;
using InstrumentId = std::uint32_t;
using Price = std::int64_t;
using Quantity = std::uint64_t;

enum class Side {
    Buy,
    Sell
};

enum class OrderType {
    Limit,
    Market
};

enum class OrderStatus {
    Active,
    PartiallyFilled,
    Filled,
    Cancelled
};

}  // namespace aegisx
