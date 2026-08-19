# AegisX Project Specification

## Purpose

AegisX is a deterministic electronic exchange simulator. It will accept order
and control events, maintain independent limit order books, match compatible
orders using price-time priority, and emit structured domain events.

The project is intended as a serious C++ systems portfolio project. The
implementation will prioritize correctness, clean architecture, testability,
determinism, and measured performance.

## Scope by phase

1. Foundation: CMake, C++20, testing, documentation, and a minimal executable.
2. Core domain model: strongly typed identifiers, prices, quantities, orders,
   and trades.
3. Price levels and order books with price priority and FIFO time priority.
4. Limit-order matching, partial fills, and multi-level matching.
5. Market orders with explicit insufficient-liquidity behavior.
6. Structured domain events.
7. Cancellation and modification semantics.
8. Multiple instruments through an exchange router.
9. Demonstration CLI.
10. Invariant checking and deterministic simulation.
11. Event recording, replay, reference implementation, benchmarking, and
    profiling.

Only Phase 0 is implemented at this point. Future phases must be added
incrementally and accompanied by tests and documentation.

## Domain decisions

- C++20 and CMake are the baseline toolchain.
- Prices will use integer ticks, never floating-point values. For example,
  150.25 can be represented as 15025 when the tick scale is one cent.
- Expected aliases include `OrderId` (`uint64_t`), `InstrumentId` (`uint32_t`),
  `Price` (`int64_t`), and `Quantity` (`uint64_t`).
- Buy priority is highest price first; sell priority is lowest price first.
- Orders at one price level are processed FIFO.
- Quantity decreases preserve priority where possible; quantity increases and
  price changes lose priority.
- Matching logic will produce domain objects/events rather than print directly.

## Non-goals for the foundation

The foundation does not include a web frontend, database, authentication,
microservices, cloud infrastructure, custom allocators, lock-free structures,
or premature optimization.

## Verification standard

Each phase must configure, compile, and pass its tests. Performance work begins
only after correctness behavior is covered and a benchmark identifies a real
bottleneck.
