# Reference Engine

`ReferenceEngine` is intentionally simple and not performance-oriented. It
stores active orders in a `std::vector<Order>` and scans linearly to select the
best opposite order. Equal-price orders retain FIFO order because the first
matching vector entry wins ties.

The reference engine implements the same limit/market matching, cancellation,
and modification semantics as the core engine. Differential tests feed both
engines identical deterministic simulation events and compare every generated
trade and final live-order state. Phase 17 extends this coverage across eight
deterministic 2,000-event workloads and checks live-book equivalence after
every event. Its purpose is to expose correctness regressions before
performance work begins.
