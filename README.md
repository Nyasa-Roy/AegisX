# AegisX

AegisX is a deterministic, event-driven electronic exchange simulator written
in modern C++. It evolves incrementally from a correctness-first domain model
into a testable and benchmarkable systems project.

## Status

Phase 13 - reference implementation. The repository contains strongly typed
domain objects, matching, events, cancellation, modification, exchange
routing, CLI/demo tools, invariants, simulation, event recording/replay, and a
simple reference engine for differential correctness checks. Benchmarking and
profiling are not implemented yet.

## Requirements

- CMake 3.20 or newer
- A C++20 compiler
- Internet access on the first test-enabled configure, so CMake can download
  the pinned GoogleTest release

## Build

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Run a simulation and replay it:

```text
build\aegisx-sim.exe --orders 100000 --seed 42 --record data\run.log
build\aegisx-replay.exe --input data\run.log
```

Run the demo or interactive CLI with `build\aegisx.exe` and
`build\aegisx-cli.exe`. On Unix-like systems, use the corresponding `./build/`
paths.

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
[docs/matching-algorithm.md](docs/matching-algorithm.md),
[docs/data-structures.md](docs/data-structures.md),
[docs/simulation.md](docs/simulation.md), and [docs/replay.md](docs/replay.md)
for design details.
