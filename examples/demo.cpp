#include <iostream>
#include <variant>
#include <vector>

#include "aegisx/exchange.hpp"
#include "aegisx/project.hpp"

int main() {
    using namespace aegisx;
    Exchange exchange;
    exchange.add_instrument(1);

    std::cout << project_name() << " demo\n";
    const std::vector<Event> first = exchange.submit_events(
        Order{1, 1, Side::Sell, OrderType::Limit, 100, Price{100}});
    const std::vector<Event> second = exchange.submit_events(
        Order{2, 1, Side::Sell, OrderType::Limit, 50, Price{101}});
    const std::vector<Event> third = exchange.submit_events(
        Order{3, 1, Side::Buy, OrderType::Limit, 120, Price{101}});
    (void)first;
    (void)second;

    for (const Event& event : third) {
        if (const auto* trade = std::get_if<TradeExecuted>(&event)) {
            std::cout << "trade: " << trade->trade.quantity << " @ "
                      << trade->trade.price << '\n';
        }
    }
    const OrderBook& book = exchange.instrument_engine(1)->book();
    std::cout << "remaining orders: " << book.order_count() << '\n';
    if (const PriceLevel* ask = book.best_ask()) {
        std::cout << "best ask: " << ask->price()
                  << " x " << ask->total_quantity() << '\n';
    }
    return 0;
}
