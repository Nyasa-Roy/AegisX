# AegisX

AegisX is a deterministic, event-driven electronic exchange simulator written
in modern C++. It evolves incrementally from a correctness-first domain model
into a testable and benchmarkable systems project.

## Status

Phase 9 - CLI and demo. The repository contains strongly typed domain objects,
FIFO price levels, limit and market matching, deterministic events,
cancellation, modification, multi-instrument routing, a simple interactive CLI,
and a complete demonstration scenario. Invariants, simulation, replay, and
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

Run the demo:

```text
build\aegisx.exe
```

Run the CLI:

```text
build\aegisx-cli.exe
```

Commands include `BUY`, `SELL`, `MARKET_BUY`, `MARKET_SELL`, `CANCEL`,
`MODIFY`, `BOOK`, `TRADES`, `HELP`, and `QUIT`. Instruments and order IDs are
numeric; submitted orders receive sequential IDs starting at 1.

On Unix-like systems, use `./build/aegisx` and `./build/aegisx-cli` instead.

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
[docs/matching-algorithm.md](docs/matching-algorithm.md), and
[docs/data-structures.md](docs/data-structures.md) for design details.
