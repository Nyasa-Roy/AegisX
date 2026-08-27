# AegisX

AegisX is a deterministic, event-driven electronic exchange simulator written
in modern C++. It evolves incrementally from a correctness-first domain model
into a testable and benchmarkable systems project.

## Status

Phase 14 - benchmarking. The repository contains strongly typed domain
objects, matching, events, cancellation, modification, exchange routing,
CLI/demo tools, invariants, deterministic simulation, replay, and Google
Benchmark workloads for insertion, matching, cancellation, and mixed events.
Profiling and optimization are not implemented yet.

## Requirements

- CMake 3.20 or newer
- A C++20 compiler
- Internet access on the first configure, so CMake can download GoogleTest and
  Google Benchmark

## Build

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run benchmarks:

```text
build\aegisx_benchmarks.exe --benchmark_min_time=0.1
```

Run simulations with `build\aegisx-sim.exe`, replay logs with
`build\aegisx-replay.exe`, and use `build\aegisx-cli.exe` for interactive
commands. On Unix-like systems, use the corresponding `./build/` paths.

## Test

```text
ctest --test-dir build --output-on-failure
```

## Engineering principles

- Correctness before optimization
- Deterministic behavior for identical input sequences
- Explicit ownership and invariants
- Tests for every feature
- Performance claims supported by measurements from the actual machine

See [docs/PROJECT_SPEC.md](docs/PROJECT_SPEC.md),
[docs/performance.md](docs/performance.md), and
[docs/reference-engine.md](docs/reference-engine.md) for design details.
