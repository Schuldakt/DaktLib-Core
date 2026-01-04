# DaktLib-Core

Header-first C++23 primitives for the DaktLib ecosystem. Provides shared interfaces, lightweight value types, and optional default runtime implementations. No third-party dependencies, deterministic behavior, and stable surface for downstream modules.

## Highlights
- Dependency-free, cross-platform (Windows, Linux, macOS)
- Interfaces for logging, allocation, events, serialization, and region lookup
- Lightweight `Result`, `Span`, and `StringView` types
- Optional defaults: `NullLogger`, `SystemAllocator` (opt-in `DAKTCORE_BUILD_IMPL`)
- C++23 features (`std::format`, concepts) with strict warning mode option

## Layout
```
DaktLib-Core/
├── include/dakt/core/
│   ├── Core.hpp
│   ├── concepts/CoreConcepts.hpp
│   ├── interfaces/{ILogger,IAllocator,IEventBus,ISerializable,IRegionProvider}.hpp
│   ├── types/{Result,Span,StringView}.hpp
│   ├── logging/NullLogger.hpp
│   └── memory/SystemAllocator.hpp
├── src/
│   ├── logging/NullLogger.cpp
│   └── memory/SystemAllocator.cpp
├── tests/unit/
├── CMakeLists.txt
├── ARCHITECTURE.md
├── README.md
└── TODO.md
```

## Build
Prereqs: CMake ≥ 4.2.1 and a C++23 compiler (MSVC 19.36+, GCC 13+, Clang 16+, AppleClang 15+).

Configure and build:
```
cmake -S . -B build
cmake --build build
```

Options (set at configure time):
- `DAKTCORE_BUILD_IMPL` (ON/OFF, default ON): build default runtime implementations under `src/`.
- `DAKTCORE_WARNINGS` (ON/OFF, default ON): enable /W4 (MSVC) or -Wall -Wextra -Wpedantic (GCC/Clang).
- `CMAKE_EXPORT_COMPILE_COMMANDS` (default ON): emit compile_commands.json.

Install headers (optional):
```
cmake --install build --prefix <dest>
```

## Usage
Link to the interface target:
```
target_link_libraries(MyTarget PRIVATE Dakt::Core)
```
Include the aggregate header:
```
#include <dakt/core/Core.hpp>
```
`DAKTCORE_BUILD_IMPL` adds `DaktCoreImpl` (static) with the default `NullLogger` and `SystemAllocator` definitions; link it only if you need those runtime units.

## Testing
Unit tests reside under `tests/unit` (framework TBD).

## Roadmap
Planned and in-progress items are tracked in TODO.md.
