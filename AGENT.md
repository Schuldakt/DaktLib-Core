# AGENT Brief — DaktLib-Core

## Mission
- Provide foundational math, memory, platform, and utility primitives for all DaktLib modules.

## Constraints
- C++23, dependency-free; cross-platform Win/Linux/macOS.
- Stable ABI surface for other modules; header-only where practical.
- Deterministic behavior; no global mutable singletons.

## Scope Highlights
- Math types, allocators, span/string utilities, platform shims, time and threading helpers.
- Error/result types consistent across modules.

## Limitations
- No third-party deps; no OS-private APIs.
- Keep compile-time footprint low; avoid template bloat.
