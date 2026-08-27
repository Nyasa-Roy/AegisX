#include "aegisx/invariants.hpp"

#include <algorithm>
#include <cstdint>
#include <unordered_set>

namespace aegisx {

InvariantReport InvariantChecker::check_order(const Order& order) {
    InvariantReport report;
    if (order.id() == 0) {
        report.violations.emplace_back("order id is zero");
    }
    if (order.instrument_id() == 0) {
        report.violations.emplace_back("order instrument id is zero");
    }
    if (order.original_quantity() == 0) {
        report.violations.emplace_back("order original quantity is zero");
    }
    if (order.executed_quantity() > order.original_quantity()) {
        report.violations.emplace_back("executed quantity exceeds original quantity");
    } else if (order.remaining_quantity() !=
               order.original_quantity() - order.executed_quantity()) {
        report.violations.emplace_back("order quantity is not conserved");
    }
    if (order.type() == OrderType::Limit) {
        if (!order.price().has_value() || order.price().value() <= 0) {
            report.violations.emplace_back("limit order does not have a positive price");
        }
    } else if (order.price().has_value()) {
        report.violations.emplace_back("market order has a price");
    }
    if (order.status() == OrderStatus::Filled && order.remaining_quantity() != 0) {
        report.violations.emplace_back("filled order has remaining quantity");
    }
    if (order.status() == OrderStatus::Active && order.executed_quantity() != 0) {
        report.violations.emplace_back("active order has executed quantity");
    }
    if (order.status() == OrderStatus::PartiallyFilled &&
        (order.executed_quantity() == 0 || order.remaining_quantity() == 0)) {
        report.violations.emplace_back("partially filled order has invalid quantities");
    }
    return report;
}

InvariantReport InvariantChecker::check_book(const OrderBook& book) {
    InvariantReport report;
    if (book.instrument_id() == 0) {
        report.violations.emplace_back("book instrument id is zero");
    }

    const std::vector<const Order*> orders = book.all_orders();
    std::unordered_set<OrderId> ids;
    for (const Order* order : orders) {
        if (order == nullptr) {
            report.violations.emplace_back("book contains a null order");
            continue;
        }
        const InvariantReport order_report = check_order(*order);
        report.violations.insert(report.violations.end(), order_report.violations.begin(),
                                 order_report.violations.end());
        if (order->instrument_id() != book.instrument_id()) {
            report.violations.emplace_back("order instrument does not match book");
        }
        if (order->type() != OrderType::Limit || !order->is_active()) {
            report.violations.emplace_back("book contains a non-active limit order violation");
        }
        if (!ids.insert(order->id()).second) {
            report.violations.emplace_back("duplicate active order id");
        }
    }
    if (orders.size() != book.order_count()) {
        report.violations.emplace_back("book order count does not match stored orders");
    }

    const PriceLevel* best_bid = book.best_bid();
    const PriceLevel* best_ask = book.best_ask();
    for (const Order* order : orders) {
        if (order->side() == Side::Buy &&
            (best_bid == nullptr || order->price().value() > best_bid->price())) {
            report.violations.emplace_back("best bid is inconsistent with book orders");
        }
        if (order->side() == Side::Sell &&
            (best_ask == nullptr || order->price().value() < best_ask->price())) {
            report.violations.emplace_back("best ask is inconsistent with book orders");
        }
    }
    if (best_bid != nullptr && best_bid->empty()) {
        report.violations.emplace_back("best bid level is empty");
    }
    if (best_ask != nullptr && best_ask->empty()) {
        report.violations.emplace_back("best ask level is empty");
    }
    return report;
}

}  // namespace aegisx
