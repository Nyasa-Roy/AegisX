# Deterministic Simulation

`OrderGenerator` is seeded with `std::mt19937_64` and receives the current
order book before generating each event. With the same seed and equivalent
book state, it produces the same sequence of submissions, cancellations, and
modifications.

Generated actions are state-aware:

- submissions create limit or market orders with deterministic IDs;
- cancellations select an order currently resting in the book;
- modifications select a current resting order and produce valid quantities,
  with occasional valid price changes.

The `aegisx-sim` executable accepts:

```text
--orders N --seed N
```

It runs the generated events through one instrument and reports aggregate
counts. It intentionally does not claim throughput or latency figures; those
belong to the later benchmark phase.
