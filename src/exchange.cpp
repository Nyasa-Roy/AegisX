#include "aegisx/exchange.hpp"

#include <stdexcept>
#include <utility>

namespace aegisx {

void Exchange::add_instrument(InstrumentId instrument_id) {
    if (instrument_id == 0) {
        throw std::invalid_argument("instrument id must be non-zero");
    }
    const auto [iterator, inserted] = engines_.try_emplace(instrument_id, instrument_id);
    (void)iterator;
    if (!inserted) {
        throw std::invalid_argument("instrument already exists");
    }
}

bool Exchange::contains_instrument(InstrumentId instrument_id) const noexcept {
    return engines_.contains(instrument_id);
}

std::size_t Exchange::instrument_count() const noexcept { return engines_.size(); }

const MatchingEngine* Exchange::instrument_engine(InstrumentId instrument_id) const noexcept {
    const auto iterator = engines_.find(instrument_id);
    return iterator == engines_.end() ? nullptr : &iterator->second;
}

std::vector<Event> Exchange::submit_events(Order order) {
    const auto iterator = engines_.find(order.instrument_id());
    if (iterator == engines_.end()) {
        throw std::invalid_argument("order instrument is not registered with the exchange");
    }
    return iterator->second.submit_events(std::move(order));
}

std::vector<Event> Exchange::cancel(InstrumentId instrument_id, OrderId order_id) {
    const auto iterator = engines_.find(instrument_id);
    if (iterator == engines_.end()) {
        throw std::invalid_argument("instrument is not registered with the exchange");
    }
    return iterator->second.cancel(order_id);
}

std::vector<Event> Exchange::modify(InstrumentId instrument_id, OrderId order_id,
                                    Quantity new_quantity, std::optional<Price> new_price) {
    const auto iterator = engines_.find(instrument_id);
    if (iterator == engines_.end()) {
        throw std::invalid_argument("instrument is not registered with the exchange");
    }
    return iterator->second.modify(order_id, new_quantity, new_price);
}

}  // namespace aegisx
