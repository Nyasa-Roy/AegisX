# AegisX

AegisX is a deterministic, event-driven electronic exchange simulator written
in modern C++. It evolves incrementally from a correctness-first domain model
into a testable and benchmarkable systems project.

## Status

Phase 12 - event recording and replay. The repository contains strongly typed
domain objects, FIFO price levels, matching, events, cancellation, modification,
multi-instrument routing, CLI/demo tools, invariants, deterministic simulation,
and stable text event-log recording/replay. Reference comparison and
benchmarking are not implemented yet.

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

Run a recorded simulation and replay it:

```text
build\aegisx-sim.exe --orders 100000 --seed 42 --record data\run.log
build\aegisx-replay.exe --input data\run.log
```

Run the demo or interactive CLI with `build\aegisx.exe` and
`build\aegisx-cli.exe`. On Unix-like systems, use `./build/aegisx-sim`,
`./build/aegisx-replay`, `./build/aegisx`, and `./build/aegisx-cli` instead.

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
[docs/data-structures.md](docs/data-structures.md), and
[docs/simulation.md](docs/simulation.md) for design details.
