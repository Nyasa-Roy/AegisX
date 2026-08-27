#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include "aegisx/exchange.hpp"

namespace {

using namespace aegisx;

class Cli {
public:
    int run() {
        std::cout << "AegisX CLI (type HELP for commands)\n";
        std::string line;
        while (std::cout << "> " && std::getline(std::cin, line)) {
            if (line.empty()) {
                continue;
            }
            try {
                if (!execute(line)) {
                    break;
                }
            } catch (const std::exception& error) {
                std::cout << "ERROR: " << error.what() << '\n';
            }
        }
        return 0;
    }

private:
    static void print_events(const std::vector<Event>& events) {
        for (const Event& event : events) {
            if (const auto* accepted = std::get_if<OrderAccepted>(&event)) {
                std::cout << "ACCEPTED order=" << accepted->order_id << '\n';
            } else if (const auto* executed = std::get_if<TradeExecuted>(&event)) {
                std::cout << "TRADE maker=" << executed->trade.maker_order_id
                          << " taker=" << executed->trade.taker_order_id
                          << " quantity=" << executed->trade.quantity
                          << " price=" << executed->trade.price << '\n';
            } else if (const auto* rested = std::get_if<OrderRested>(&event)) {
                std::cout << "RESTED order=" << rested->order_id
                          << " remaining=" << rested->remaining_quantity << '\n';
            } else if (const auto* cancelled = std::get_if<OrderCancelled>(&event)) {
                std::cout << "CANCELLED order=" << cancelled->order_id << '\n';
            } else if (const auto* modified = std::get_if<OrderModified>(&event)) {
                std::cout << "MODIFIED order=" << modified->order_id
                          << " quantity=" << modified->original_quantity
                          << " price=" << modified->price
                          << " priority_reset=" << std::boolalpha << modified->priority_reset
                          << std::noboolalpha << '\n';
            }
        }
    }

    void ensure_instrument(InstrumentId instrument_id) {
        if (!exchange_.contains_instrument(instrument_id)) {
            exchange_.add_instrument(instrument_id);
        }
    }

    bool execute(const std::string& line) {
        std::istringstream input(line);
        std::string command;
        input >> command;
        if (command == "QUIT" || command == "EXIT") {
            return false;
        }
        if (command == "HELP") {
            std::cout << "BUY <instrument> <quantity> <price>\n"
                      << "SELL <instrument> <quantity> <price>\n"
                      << "MARKET_BUY <instrument> <quantity>\n"
                      << "MARKET_SELL <instrument> <quantity>\n"
                      << "CANCEL <instrument> <order_id>\n"
                      << "MODIFY <instrument> <order_id> <quantity> [price]\n"
                      << "BOOK <instrument>\nTRADES\nQUIT\n";
            return true;
        }
        if (command == "TRADES") {
            for (const Trade& trade : trades_) {
                std::cout << "TRADE maker=" << trade.maker_order_id
                          << " taker=" << trade.taker_order_id
                          << " quantity=" << trade.quantity
                          << " price=" << trade.price << '\n';
            }
            return true;
        }
        if (command == "BOOK") {
            InstrumentId instrument_id{};
            input >> instrument_id;
            const MatchingEngine* engine = exchange_.instrument_engine(instrument_id);
            if (engine == nullptr) {
                throw std::invalid_argument("instrument is not registered");
            }
            const OrderBook& book = engine->book();
            std::cout << "BOOK instrument=" << instrument_id
                      << " orders=" << book.order_count() << '\n';
            if (const PriceLevel* bid = book.best_bid()) {
                std::cout << "BEST_BID price=" << bid->price()
                          << " quantity=" << bid->total_quantity() << '\n';
            }
            if (const PriceLevel* ask = book.best_ask()) {
                std::cout << "BEST_ASK price=" << ask->price()
                          << " quantity=" << ask->total_quantity() << '\n';
            }
            return true;
        }

        InstrumentId instrument_id{};
        input >> instrument_id;
        if (command == "BUY" || command == "SELL") {
            Quantity quantity{};
            Price price{};
            input >> quantity >> price;
            ensure_instrument(instrument_id);
            const Side side = command == "BUY" ? Side::Buy : Side::Sell;
            print_and_record(exchange_.submit_events(
                Order{next_order_id_++, instrument_id, side, OrderType::Limit, quantity, price}));
            return true;
        }
        if (command == "MARKET_BUY" || command == "MARKET_SELL") {
            Quantity quantity{};
            input >> quantity;
            ensure_instrument(instrument_id);
            const Side side = command == "MARKET_BUY" ? Side::Buy : Side::Sell;
            print_and_record(exchange_.submit_events(
                Order{next_order_id_++, instrument_id, side, OrderType::Market, quantity}));
            return true;
        }
        if (command == "CANCEL") {
            OrderId order_id{};
            input >> order_id;
            print_events(exchange_.cancel(instrument_id, order_id));
            return true;
        }
        if (command == "MODIFY") {
            OrderId order_id{};
            Quantity quantity{};
            std::optional<Price> price;
            input >> order_id >> quantity;
            Price parsed_price{};
            if (input >> parsed_price) {
                price = parsed_price;
            }
            print_events(exchange_.modify(instrument_id, order_id, quantity, price));
            return true;
        }
        throw std::invalid_argument("unknown command; type HELP");
    }

    void print_and_record(const std::vector<Event>& events) {
        print_events(events);
        for (const Event& event : events) {
            if (const auto* executed = std::get_if<TradeExecuted>(&event)) {
                trades_.push_back(executed->trade);
            }
        }
    }

    Exchange exchange_;
    std::vector<Trade> trades_;
    OrderId next_order_id_{1};
};

}  // namespace

int main() {
    return Cli{}.run();
}
