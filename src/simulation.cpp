#include "aegisx/simulation.hpp"

#include <stdexcept>

namespace aegisx {

OrderGenerator::OrderGenerator(std::uint64_t seed, InstrumentId instrument_id)
    : random_engine_(seed), instrument_id_(instrument_id) {
    if (instrument_id_ == 0) {
        throw std::invalid_argument("simulation instrument id must be non-zero");
    }
}

SimulationEvent OrderGenerator::next(const OrderBook& book) {
    const std::vector<const Order*> active_orders = book.all_orders();
    std::uniform_int_distribution<unsigned> action_distribution(0, 99);
    const unsigned action = action_distribution(random_engine_);
    if (active_orders.empty() || action < 60) {
        return make_submit();
    }
    if (action < 80) {
        return make_cancel(active_orders);
    }
    return make_modify(active_orders);
}

SimulationEvent OrderGenerator::make_submit() {
    std::uniform_int_distribution<unsigned> side_distribution(0, 1);
    std::uniform_int_distribution<unsigned> type_distribution(0, 4);
    std::uniform_int_distribution<Quantity> quantity_distribution(1, 100);
    const Side side = side_distribution(random_engine_) == 0 ? Side::Buy : Side::Sell;
    const Quantity quantity = quantity_distribution(random_engine_);
    const OrderId order_id = next_order_id_++;
    if (type_distribution(random_engine_) == 0) {
        return SubmitEvent{Order{order_id, instrument_id_, side, OrderType::Market, quantity}};
    }

    std::uniform_int_distribution<Price> price_distribution(95, 105);
    return SubmitEvent{Order{order_id, instrument_id_, side, OrderType::Limit, quantity,
                             price_distribution(random_engine_)}};
}

SimulationEvent OrderGenerator::make_cancel(
    const std::vector<const Order*>& active_orders) {
    std::uniform_int_distribution<std::size_t> index_distribution(0, active_orders.size() - 1);
    return CancelEvent{active_orders[index_distribution(random_engine_)]->id()};
}

SimulationEvent OrderGenerator::make_modify(
    const std::vector<const Order*>& active_orders) {
    std::uniform_int_distribution<std::size_t> index_distribution(0, active_orders.size() - 1);
    const Order& order = *active_orders[index_distribution(random_engine_)];
    Quantity new_quantity = order.original_quantity();
    std::uniform_int_distribution<unsigned> change_distribution(0, 2);
    if (change_distribution(random_engine_) == 0 && order.original_quantity() > order.executed_quantity() + 1) {
        std::uniform_int_distribution<Quantity> decrease_distribution(
            order.executed_quantity() + 1, order.original_quantity() - 1);
        new_quantity = decrease_distribution(random_engine_);
    } else {
        std::uniform_int_distribution<Quantity> increase_distribution(1, 50);
        new_quantity += increase_distribution(random_engine_);
    }

    std::optional<Price> new_price;
    std::uniform_int_distribution<unsigned> price_change_distribution(0, 3);
    if (price_change_distribution(random_engine_) == 0) {
        const Price delta = (change_distribution(random_engine_) == 0) ? Price{1} : Price{-1};
        new_price = order.price().value() + delta;
    }
    return ModifyEvent{order.id(), new_quantity, new_price};
}

}  // namespace aegisx
