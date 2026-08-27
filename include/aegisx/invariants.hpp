#pragma once

#include <string>
#include <vector>

#include "aegisx/order.hpp"
#include "aegisx/order_book.hpp"

namespace aegisx {

struct InvariantReport {
    std::vector<std::string> violations;

    [[nodiscard]] bool valid() const noexcept { return violations.empty(); }
};

class InvariantChecker {
public:
    [[nodiscard]] static InvariantReport check_order(const Order& order);
    [[nodiscard]] static InvariantReport check_book(const OrderBook& book);
};

}  // namespace aegisx
