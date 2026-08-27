#pragma once

#include <optional>

#include "aegisx/types.hpp"

namespace aegisx {

class Order {
public:
    Order(OrderId id,
          InstrumentId instrument_id,
          Side side,
          OrderType type,
          Quantity quantity,
          std::optional<Price> price = std::nullopt);

    [[nodiscard]] OrderId id() const noexcept;
    [[nodiscard]] InstrumentId instrument_id() const noexcept;
    [[nodiscard]] Side side() const noexcept;
    [[nodiscard]] OrderType type() const noexcept;
    [[nodiscard]] std::optional<Price> price() const noexcept;
    [[nodiscard]] Quantity original_quantity() const noexcept;
    [[nodiscard]] Quantity executed_quantity() const noexcept;
    [[nodiscard]] Quantity remaining_quantity() const noexcept;
    [[nodiscard]] OrderStatus status() const noexcept;
    [[nodiscard]] bool is_active() const noexcept;

    void execute(Quantity quantity);
    void cancel();
    void modify(Quantity new_quantity, std::optional<Price> new_price = std::nullopt);

private:
    OrderId id_;
    InstrumentId instrument_id_;
    Side side_;
    OrderType type_;
    std::optional<Price> price_;
    Quantity original_quantity_;
    Quantity executed_quantity_{0};
    Quantity remaining_quantity_;
    OrderStatus status_{OrderStatus::Active};
};

}  // namespace aegisx
