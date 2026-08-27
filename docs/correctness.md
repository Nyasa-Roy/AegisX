# Correctness and Invariants

`InvariantChecker` validates individual orders and live order books without
mutating them.

For every order it checks:

- identifiers and original quantity are non-zero;
- executed quantity does not exceed original quantity;
- `original = executed + remaining`;
- limit orders have positive prices and market orders have no price;
- filled, active, and partially filled statuses agree with quantities.

For every order book it additionally checks instrument ownership, active limit
orders only, duplicate active IDs, stored order count, non-empty best levels,
and consistency of best bid/ask prices with all live orders.

The checker returns an `InvariantReport` containing every detected violation so
tests and future diagnostic tools can report multiple failures at once.
