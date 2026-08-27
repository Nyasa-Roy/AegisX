#include "aegisx/replay.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace aegisx {

void EventLog::write(std::ostream& output, const std::vector<SimulationEvent>& events) {
    for (const SimulationEvent& event : events) {
        std::visit([&output](const auto& typed_event) {
            using EventType = std::decay_t<decltype(typed_event)>;
            if constexpr (std::is_same_v<EventType, SubmitEvent>) {
                const Order& order = typed_event.order;
                output << "SUBMIT " << order.id() << ' ' << order.instrument_id() << ' '
                       << static_cast<unsigned>(order.side()) << ' '
                       << static_cast<unsigned>(order.type()) << ' '
                       << order.original_quantity() << ' '
                       << order.price().value_or(Price{0}) << '\n';
            } else if constexpr (std::is_same_v<EventType, CancelEvent>) {
                output << "CANCEL " << typed_event.order_id << '\n';
            } else {
                output << "MODIFY " << typed_event.order_id << ' '
                       << typed_event.new_quantity << ' '
                       << typed_event.new_price.value_or(Price{0}) << '\n';
            }
        }, event);
    }
}

std::vector<SimulationEvent> EventLog::read(std::istream& input) {
    std::vector<SimulationEvent> events;
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        if (line.empty()) {
            continue;
        }

        std::istringstream parser(line);
        std::string command;
        parser >> command;
        if (command == "SUBMIT") {
            std::uint64_t order_id{};
            std::uint64_t instrument_id{};
            unsigned side{};
            unsigned type{};
            Quantity quantity{};
            Price encoded_price{};
            if (!(parser >> order_id >> instrument_id >> side >> type >> quantity >> encoded_price) ||
                side > 1U || type > 1U) {
                throw std::invalid_argument("invalid SUBMIT event at line " + std::to_string(line_number));
            }
            const Side parsed_side = side == 0U ? Side::Buy : Side::Sell;
            const OrderType parsed_type = type == 0U ? OrderType::Limit : OrderType::Market;
            const std::optional<Price> price = parsed_type == OrderType::Limit
                ? std::optional<Price>{encoded_price}
                : std::nullopt;
            events.emplace_back(SubmitEvent{Order{static_cast<OrderId>(order_id),
                                                  static_cast<InstrumentId>(instrument_id),
                                                  parsed_side, parsed_type, quantity, price}});
        } else if (command == "CANCEL") {
            OrderId order_id{};
            if (!(parser >> order_id)) {
                throw std::invalid_argument("invalid CANCEL event at line " + std::to_string(line_number));
            }
            events.emplace_back(CancelEvent{order_id});
        } else if (command == "MODIFY") {
            OrderId order_id{};
            Quantity quantity{};
            Price encoded_price{};
            if (!(parser >> order_id >> quantity >> encoded_price)) {
                throw std::invalid_argument("invalid MODIFY event at line " + std::to_string(line_number));
            }
            const std::optional<Price> price = encoded_price == 0
                ? std::nullopt
                : std::optional<Price>{encoded_price};
            events.emplace_back(ModifyEvent{order_id, quantity, price});
        } else {
            throw std::invalid_argument("unknown event at line " + std::to_string(line_number));
        }
    }
    return events;
}

void EventLog::write_file(const std::string& path, const std::vector<SimulationEvent>& events) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("unable to open event log for writing: " + path);
    }
    write(output, events);
}

std::vector<SimulationEvent> EventLog::read_file(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("unable to open event log for reading: " + path);
    }
    return read(input);
}

}  // namespace aegisx
