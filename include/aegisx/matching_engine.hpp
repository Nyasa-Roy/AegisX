#pragma once

#include <vector>

#include "aegisx/event.hpp"
#include "aegisx/order_book.hpp"
#include "aegisx/trade.hpp"

namespace aegisx {

class MatchingEngine {
public:
    explicit MatchingEngine(InstrumentId instrument_id);

    [[nodiscard]] InstrumentId instrument_id() const noexcept;
    [[nodiscard]] const OrderBook& book() const noexcept;

    std::vector<Trade> submit(Order order);
    std::vector<Event> submit_events(Order order);

private:
    OrderBook book_;
};

}  // namespace aegisx
