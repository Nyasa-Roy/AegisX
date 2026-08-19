# AegisX

AegisX is a deterministic, event-driven electronic exchange simulator written
in modern C++. It will evolve incrementally from a correctness-first domain
model into a testable and benchmarkable systems project.

## Status

Phase 0 — project foundation. The repository currently contains the CMake
build, a minimal library and demo executable, GoogleTest integration, and a
smoke test. Order books, orders, and matching are intentionally not implemented
yet.

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

On Unix-like systems, use `./build/aegisx` instead.

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

See [docs/PROJECT_SPEC.md](docs/PROJECT_SPEC.md) for the phased scope and
domain requirements.
