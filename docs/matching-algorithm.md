# Matching Algorithm

## Rule

AegisX uses price-time priority for limit orders:

- Buy orders match the lowest available sell price first.
- Sell orders match the highest available buy price first.
- Orders at the same price are matched in insertion order.
- Trades execute at the resting order's price.

## Process

When an order arrives, the matching engine repeatedly examines the best
opposite-side order. If a limit order's prices cross, or if the incoming order
is a market order, it executes the smaller remaining quantity and emits one
`Trade`. A filled resting order is removed from the front of its price level.
The process stops when the incoming order is filled, the opposite book has no
liquidity, or a limit order's best opposite price no longer crosses.

Any remaining incoming quantity is inserted into its own side of the book,
preserving its time priority from that point onward.

## Example

With sells of 50 at 100, 50 at 101, and 50 at 102, an incoming buy of 120 at
102 produces trades of:

```text
50 at 100
50 at 101
20 at 102
```

The final remaining sell quantity at 102 is 30.

## Market orders

Market orders consume available liquidity without a price constraint. They
execute at each resting order's price and never enter the book. If available
liquidity is smaller than the requested quantity, the unfilled remainder is
discarded when submission ends. An empty opposite book therefore produces no
trades and leaves the book unchanged.

## Events

`submit_events` emits a deterministic sequence:

1. `OrderAccepted` for the incoming order.
2. One `TradeExecuted` for each fill, in matching order.
3. `OrderRested` when an unfilled limit-order remainder is added to the book.

An unfilled market-order remainder produces no resting event because it is
discarded by the explicit Phase 4 market-order policy.

Cancellation removes an active order from its price level, emits one
`OrderCancelled` event, updates the order-ID index, and removes the price level
when it becomes empty. Filled, cancelled, and unknown orders cannot be
cancelled through the matching engine.
