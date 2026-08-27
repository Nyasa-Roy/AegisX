#pragma once

#include <cstddef>
#include <map>
#include <vector>

#include "aegisx/matching_engine.hpp"

namespace aegisx {

class Exchange {
public:
    void add_instrument(InstrumentId instrument_id);

    [[nodiscard]] bool contains_instrument(InstrumentId instrument_id) const noexcept;
    [[nodiscard]] std::size_t instrument_count() const noexcept;
    [[nodiscard]] const MatchingEngine* instrument_engine(InstrumentId instrument_id) const noexcept;

    std::vector<Event> submit_events(Order order);
    std::vector<Event> cancel(InstrumentId instrument_id, OrderId order_id);
    std::vector<Event> modify(InstrumentId instrument_id, OrderId order_id,
                              Quantity new_quantity,
                              std::optional<Price> new_price = std::nullopt);

private:
    std::map<InstrumentId, MatchingEngine> engines_;
};

}  // namespace aegisx
