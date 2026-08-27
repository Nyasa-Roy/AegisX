# Performance and Benchmarking

Phase 14 adds Google Benchmark workloads for:

- order-book insertion;
- limit-book matching through market orders;
- cancellation across a price level;
- mixed seeded simulation events.

Run the benchmark executable on the target machine and preserve its output
alongside the compiler, build type, operating system, CPU, and memory details.
The benchmark suite currently records measurements but does not claim a
performance target. Optimization begins only after profiling identifies a
measurable bottleneck and a before/after comparison is recorded.

## Baseline snapshot

Measured on 2026-08-27 in a Release build on Windows using MSYS2 UCRT64,
GCC 14.2.0, and Google Benchmark 1.8.3. The machine reported 16 logical CPUs
at 1797 MHz, with 32 KiB L1 data/instruction caches per core, 512 KiB L2 per
core, and 4096 KiB L3 per package. These are single-iteration measurements
(`--benchmark_min_time=1x`) and are a baseline, not a performance claim.

```text
benchmark_order_book_insert/100       86200 ns
benchmark_order_book_insert/1000     455000 ns
benchmark_matching/100               255700 ns
benchmark_matching/1000             3174500 ns
benchmark_cancel/100                 174500 ns
benchmark_cancel/1000               2803500 ns
benchmark_mixed_workload/1000       1423400 ns
benchmark_mixed_workload/10000     13306500 ns
```

The benchmark executable should be rerun after any optimization under the same
or explicitly documented conditions.
