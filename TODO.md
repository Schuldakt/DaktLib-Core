# Core Module TODO

## Status: ✅ Implementation Complete

The Core module is feature-complete and ready for use. Items below are enhancements and optimizations.

### Core Headers
- `Types.hpp` - Fundamental type aliases (i8-i64, u8-u64, f32, f64, byte, String, Option, Result)
- `Macros.hpp` - DAKT_ASSERT, DAKT_UNREACHABLE, platform detection
- `Memory.hpp` - Custom allocators (Linear, Pool, Arena)
- `String.hpp` - String utilities (format, split, trim, case conversion)
- `Buffer.hpp` - Byte buffer with read/write cursor
- `Hash.hpp` - FNV-1a, CRC32, xxHash algorithms
- `Time.hpp` - Clock, Timer, duration utilities
- `FileSystem.hpp` - Path manipulation, file I/O
- `Platform.hpp` - OS abstractions (user paths, DLL loading)
- `Geometry.hpp` - Vec2, Vec4, Color, Rect, Circle (shared geometry primitives)

---

## High Priority

### Testing
- [ ] Unit tests for all Types.hpp functionality
- [ ] Unit tests for Memory allocators
- [ ] Unit tests for String utilities
- [ ] Unit tests for Buffer read/write operations
- [ ] Unit tests for Hash functions (compare against reference implementations)
- [ ] Unit tests for Time utilities
- [ ] Unit tests for FileSystem operations
- [ ] Unit tests for Platform functions
- [ ] Fuzz testing for string parsing functions
- [ ] Fuzz testing for buffer reader edge cases

### Documentation
- [ ] Add Doxygen comments to all public APIs
- [ ] Add usage examples to each header file
- [ ] Create getting started guide

### Build System
- [ ] Verify MSVC 19.35+ compilation
- [ ] Verify GCC 13+ compilation  
- [ ] Verify Clang 16+ compilation
- [ ] Add CI/CD configuration (GitHub Actions)
- [ ] Add code coverage reporting

---

## Medium Priority

### Types.hpp
- [ ] Add `Expected<T>` alias for `Result<T, Error>` (more intuitive name)
- [ ] Consider `Result<T, std::error_code>` variant for OS errors
- [ ] Add `try_` macros for result propagation (similar to Rust's `?`)

### Memory.hpp
- [ ] Add `StackAllocator` for small, scoped allocations
- [ ] Add memory debugging/leak detection in debug builds
- [ ] Add allocation source tracking (file/line) in debug builds
- [ ] Implement proper save/restore for `ScopedAllocator`
- [ ] Add `allocate_unique<T>()` and `allocate_shared<T>()` with custom allocator
- [ ] Thread-safe arena allocator variant

### String.hpp
- [ ] Add `StringView` tokenizer/iterator
- [ ] Add regex wrapper (for pattern matching)
- [ ] Add fuzzy string matching (Levenshtein distance)
- [ ] Add natural sort comparison
- [ ] Add CSV parsing utilities
- [ ] Optimize `replace()` for large strings with many replacements
- [ ] Add `split()` variant that returns a generator/range

### Buffer.hpp
- [ ] Add bit-level read/write operations
- [ ] Add variable-length integer encoding (varint, LEB128)
- [ ] Add compression/decompression integration hooks
- [ ] Add checksum validation during read
- [ ] Add structured binding support for reader operations
- [ ] Pool-allocated buffer for reduced heap pressure

### Hash.hpp
- [ ] Add streaming/incremental hash APIs for all algorithms
- [ ] Add SIMD-optimized implementations (SSE4.2, AVX2)
- [ ] Add hardware CRC32 acceleration (SSE4.2 `_mm_crc32_*`)
- [ ] Add hardware AES acceleration for future crypto module
- [ ] Benchmark and tune hash table load factors

### Time.hpp
- [ ] Add timezone support (C++20 chrono zones)
- [ ] Add duration formatting with custom formats
- [ ] Add calendar operations (add days, months, years)
- [ ] Add parsing of timestamp strings
- [ ] Add monotonic vs wall clock distinction
- [ ] Improve frame timer with jitter smoothing

### FileSystem.hpp
- [ ] Add async file operations (coroutines)
- [ ] Add file locking (exclusive/shared)
- [ ] Add atomic file write (write to temp, rename)
- [ ] Add directory change watching
- [ ] Add symbolic link handling
- [ ] Add file permission utilities
- [ ] Add disk space queries
- [ ] Improve glob pattern matching

### Platform.hpp
- [ ] Add CPU feature detection (SSE, AVX, etc.)
- [ ] Add GPU detection (for future compute features)
- [ ] Add crash handler / minidump support
- [ ] Add stack trace capture
- [ ] Add system locale information
- [ ] Add high-DPI awareness utilities

---

## Low Priority / Future

### Performance
- [ ] Profile all hot paths
- [ ] Add compile-time string formatting where possible
- [ ] Consider `small_vector` for small buffer optimization
- [ ] SIMD string search (`memmem` replacement)
- [ ] Memory-mapped file caching strategy

### Compatibility
- [ ] Linux/macOS full parity testing
- [ ] MinGW support
- [ ] Emscripten/WASM exploration

### Features
- [ ] Reflection/introspection utilities
- [ ] Serialization framework hooks
- [ ] JSON pointer/path support in String
- [ ] XML escape/unescape utilities
- [ ] URL encoding/decoding

---

## Known Issues

1. **FileSystem glob()** - Not yet implemented, returns empty result
2. **MemoryMappedFile** - Windows implementation only, Linux/macOS stubbed
3. **utf8::toWide()** - Non-Windows implementation is basic (doesn't handle all edge cases)

---

## Completed ✓

- [x] Macros.hpp - All assertions, compiler detection, utilities
- [x] Types.hpp - Result, Option, Span, concepts, integer aliases
- [x] Platform.hpp - OS detection, paths, environment, byte swapping
- [x] Memory.hpp - HeapAllocator, ArenaAllocator, PoolAllocator, Ref<T>
- [x] String.hpp - All string utilities, encoding, parsing
- [x] Buffer.hpp - Buffer, BufferReader, BufferWriter
- [x] Hash.hpp - FNV, CRC32, XXHash, MurmurHash
- [x] Time.hpp - Stopwatch, Timer, FrameTimer, Timestamp
- [x] FileSystem.hpp - Path utilities, file I/O, directory operations
- [x] CMakeLists.txt - Build configuration
- [x] ARCHITECTURE.md - Module documentation

---

## Notes

### C++23 Feature Usage

The Core module uses these C++23 features:
- `std::expected` - Result type
- `std::optional` monadic operations - `.and_then()`, `.transform()`
- `[[assume]]` attribute - Optimization hints
- `std::unreachable()` - Unreachable code marker
- `std::byteswap()` - Byte swapping
- `consteval` - Compile-time hash functions
- `std::to_underlying()` - Enum conversion

Fallbacks are NOT provided - C++23 is required.

### Breaking Changes Policy

During initial development (v0.x), breaking changes may occur without deprecation. After v1.0:
- Deprecated APIs will remain for at least one minor version
- Removed APIs will be documented in CHANGELOG
- Binary compatibility is NOT guaranteed between any versions