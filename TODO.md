# DaktLib-Core TODO

## Legend
- **Priority**: P0 (Critical) → P3 (Nice-to-have)
- **Complexity**: S (Small, <2h) | M (Medium, 2-8h) | L (Large, 8h+)

---

## P0 — Foundation

### Project Setup
- [ ] **[S]** Create directory structure (`include/dakt/core/...`)
- [ ] **[S]** Setup `CMakeLists.txt` (INTERFACE library, install rules)
- [ ] **[S]** Add `.clang-format` configuration
- [ ] **[S]** Add basic README.md

### Core Types
- [ ] **[M]** Implement `Result<T, E>` with monadic ops (`map`, `andThen`, `orElse`)
- [ ] **[S]** Implement `Result<void, E>` specialization
- [ ] **[S]** Implement `Span<T>` (non-owning view)
- [ ] **[S]** Implement `StringView` (null-safe, constexpr)

### Logging Interface
- [ ] **[S]** Define `Severity` enum for logging levels
- [ ] **[S]** Define `ILogger` interface
- [ ] **[S]** Implement `NullLogger` default

### Memory Interface
- [ ] **[S]** Define `IAllocator` interface
- [ ] **[S]** Implement `SystemAllocator` default

### Aggregate Header
- [ ] **[S]** Create aggregate header `Core.hpp`

---

## P1 — Core Interfaces

### Event System
- [ ] **[M]** Define `IEventBus` interface (publish/subscribe pattern)
- [ ] **[S]** Define `EventId` and `SubscriptionToken` types

### Serialization
- [ ] **[M]** Define `ISerializable` interface (byte-based contract)

### Region System
- [ ] **[S]** Define `Rect` struct for screen regions
- [ ] **[S]** Define `IRegionProvider` interface

### Concepts
- [ ] **[M]** Add C++20/23 concepts (`Loggable`, `Serializable`, `Allocatable`)
- [ ] **[S]** Add `RegionProvider` concept

### Quality
- [ ] **[S]** Add `[[nodiscard]]` annotations throughout
- [ ] **[S]** Add `constexpr` where applicable
- [ ] **[S]** Add `noexcept` specifications

---

## P2 — Testing & Quality

### Unit Tests
- [ ] **[M]** Setup Catch2/doctest for unit tests
- [ ] **[M]** Write `Result<T,E>` unit tests (ok/err paths, monadic chaining)
- [ ] **[S]** Write `Span<T>` unit tests (bounds, subspan)
- [ ] **[S]** Write `StringView` unit tests (construction, comparison, find)
- [ ] **[L]** Add compile-time tests (static_assert for constexpr correctness)

### CI/CD
- [ ] **[M]** Add GitHub Actions workflow (MSVC, GCC, Clang)
- [ ] **[S]** Add clang-format check to CI
- [ ] **[S]** Add clang-tidy check to CI

### Code Quality
- [ ] **[S]** Add clang-tidy configuration
- [ ] **[M]** Address all clang-tidy warnings

---

## P3 — Enhancements

### Convenience Types
- [ ] **[M]** Add `Expected<T>` alias (`Result<T, std::string>`)
- [ ] **[M]** Add `TypeId` utility for event type hashing
- [ ] **[S]** Add `ScopeGuard` RAII utility

### Source Location
- [ ] **[L]** Add optional `std::source_location` support in `ILogger`

### Documentation
- [ ] **[M]** Add Doxygen documentation comments
- [ ] **[S]** Create `CHANGELOG.md`
- [ ] **[M]** Write integration example (minimal consumer project)

### Optional Features
- [ ] **[M]** Add `Optional<T>` wrapper with monadic operations
- [ ] **[S]** Add `Either<L, R>` type
- [ ] **[M]** Add `Lazy<T>` deferred initialization

---

## Milestones

| Milestone | Target | Tasks |
|-----------|--------|-------|
| **v0.1.0** | Week 1 | P0 complete (core types) |
| **v0.2.0** | Week 2 | P0 + P1 complete (all interfaces) |
| **v0.3.0** | Week 3 | P0 + P1 + P2 complete (tested) |
| **v1.0.0** | Week 4 | All phases complete (stable release) |

---

## Acceptance Criteria

### Result<T, E>
- [ ] Can construct with `Result::ok(value)` and `Result::err(error)`
- [ ] `isOk()` and `isErr()` work correctly
- [ ] `value()` returns reference to stored value
- [ ] `error()` returns reference to stored error
- [ ] `map()` transforms success value
- [ ] `andThen()` chains operations
- [ ] `orElse()` handles errors
- [ ] `valueOr()` provides default on error
- [ ] Works with move-only types
- [ ] `Result<void, E>` specialization works

### Span<T>
- [ ] Default constructor creates empty span
- [ ] Can construct from pointer + size
- [ ] Can construct from C array
- [ ] Can construct from std::vector, std::array
- [ ] `data()`, `size()`, `empty()` work correctly
- [ ] `operator[]` provides element access
- [ ] `subspan()` creates sub-ranges
- [ ] Range-based for loop works

### StringView
- [ ] Default constructor creates empty view
- [ ] Can construct from const char*
- [ ] Can construct from std::string
- [ ] `data()`, `size()`, `length()`, `empty()` work
- [ ] `substr()` creates sub-views
- [ ] `find()` locates characters and substrings
- [ ] Comparison operators work
- [ ] Converts to std::string_view

### Interfaces
- [ ] `ILogger` can be implemented by external code
- [ ] `IAllocator` can be implemented by external code
- [ ] `IEventBus` can be implemented by external code
- [ ] `ISerializable` can be implemented by external code
- [ ] `IRegionProvider` can be implemented by external code
- [ ] Default implementations work correctly
