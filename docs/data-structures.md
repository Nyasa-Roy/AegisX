# Data Structures

## PriceLevel

`PriceLevel` stores one positive integer price and a `std::deque<Order>`.
Orders are appended to the deque, so insertion preserves FIFO time priority at
that price. The level exposes read-only access to its order sequence and total
remaining quantity.

## OrderBook

`OrderBook` owns two independent ordered maps:

- Bids: `std::map<Price, PriceLevel, std::greater<Price>>`
- Asks: `std::map<Price, PriceLevel>`

The first bid map entry is therefore the highest bid, while the first ask map
entry is the lowest ask. An unordered set tracks order IDs currently inserted
in the book and prevents duplicate resting orders.

The book owns the `Order` values. Best-level access returns a read-only pointer
valid until the book is structurally modified; callers must not retain it
across future insertions or later cancellation/matching operations.

## Complexity

- Insert into a price level: amortized constant deque insertion after map lookup
- Insert into the book: `O(log P)` for `P` price levels, plus expected constant ID lookup
- Best bid/ask lookup: `O(1)`
- FIFO front lookup: `O(1)`

This is the simplest correct Phase 2 structure. Allocation and cache behavior
will be measured before any future optimization is considered.
