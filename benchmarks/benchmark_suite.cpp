#include <cstddef>
#include <type_traits>
#include <variant>

#include <benchmark/benchmark.h>

#include "aegisx/matching_engine.hpp"
#include "aegisx/order_book.hpp"
#include "aegisx/simulation.hpp"

namespace {

using namespace aegisx;

void benchmark_order_book_insert(benchmark::State& state) {
    const auto order_count = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        OrderBook book{1};
        for (std::size_t index = 0; index < order_count; ++index) {
            book.add_order(Order{static_cast<OrderId>(index + 1), 1, Side::Buy,
                                 OrderType::Limit, 1, Price{100}});
        }
        benchmark::DoNotOptimize(book.order_count());
    }
}

void benchmark_matching(benchmark::State& state) {
    const auto order_count = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        MatchingEngine engine{1};
        for (std::size_t index = 0; index < order_count; ++index) {
            engine.submit(Order{static_cast<OrderId>(index + 1), 1, Side::Sell,
                                OrderType::Limit, 1, Price{100 + static_cast<Price>(index)}});
        }
        engine.submit(Order{static_cast<OrderId>(order_count + 1), 1, Side::Buy,
                            OrderType::Market, static_cast<Quantity>(order_count)});
        benchmark::DoNotOptimize(engine.book().order_count());
    }
}

void benchmark_cancel(benchmark::State& state) {
    const auto order_count = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        MatchingEngine engine{1};
        for (std::size_t index = 0; index < order_count; ++index) {
            engine.submit(Order{static_cast<OrderId>(index + 1), 1, Side::Buy,
                                OrderType::Limit, 1, Price{100}});
        }
        for (std::size_t index = 0; index < order_count; index += 2) {
            engine.cancel(static_cast<OrderId>(index + 1));
        }
        benchmark::DoNotOptimize(engine.book().order_count());
    }
}

void apply_event(MatchingEngine& engine, const SimulationEvent& event) {
    std::visit([&](const auto& typed_event) {
        using EventType = std::decay_t<decltype(typed_event)>;
        if constexpr (std::is_same_v<EventType, SubmitEvent>) {
            engine.submit_events(typed_event.order);
        } else if constexpr (std::is_same_v<EventType, CancelEvent>) {
            engine.cancel(typed_event.order_id);
        } else {
            engine.modify(typed_event.order_id, typed_event.new_quantity, typed_event.new_price);
        }
    }, event);
}

void benchmark_mixed_workload(benchmark::State& state) {
    const auto event_count = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        MatchingEngine engine{1};
        OrderGenerator generator{42};
        for (std::size_t index = 0; index < event_count; ++index) {
            apply_event(engine, generator.next(engine.book()));
        }
        benchmark::DoNotOptimize(engine.book().order_count());
    }
}

BENCHMARK(benchmark_order_book_insert)->Arg(100)->Arg(1000);
BENCHMARK(benchmark_matching)->Arg(100)->Arg(1000);
BENCHMARK(benchmark_cancel)->Arg(100)->Arg(1000);
BENCHMARK(benchmark_mixed_workload)->Arg(1000)->Arg(10000);

}  // namespace
