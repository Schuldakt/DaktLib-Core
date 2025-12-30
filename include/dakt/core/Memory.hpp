#pragma once

// ============================================================================
// DaktLib Core - Memory
// Memory utilities and allocators using C++23 features
// ============================================================================

#include "Macros.hpp"
#include "Platform.hpp"
#include "Types.hpp"

#include <atomic>
#include <concepts>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <memory_resource>

namespace dakt::core
{

// ============================================================================
// Alignment Utilities (using C++20 <bit>)
// ============================================================================

// Align value up to alignment boundary
[[nodiscard]] DAKT_FORCEINLINE constexpr usize alignUp(usize value, usize alignment) noexcept
{
    DAKT_ASSERT(isPowerOfTwo(alignment));
    return (value + alignment - 1) & ~(alignment - 1);
}

// Align value down to alignment boundary
[[nodiscard]] DAKT_FORCEINLINE constexpr usize alignDown(usize value, usize alignment) noexcept
{
    DAKT_ASSERT(isPowerOfTwo(alignment));
    return value & ~(alignment - 1);
}

// Check if pointer is aligned
[[nodiscard]] DAKT_FORCEINLINE bool isAligned(const void* ptr, usize alignment) noexcept
{
    return (reinterpret_cast<usize>(ptr) & (alignment - 1)) == 0;
}

// Align pointer up
[[nodiscard]] DAKT_FORCEINLINE void* alignPtr(void* ptr, usize alignment) noexcept
{
    return reinterpret_cast<void*>(alignUp(reinterpret_cast<usize>(ptr), alignment));
}

// ============================================================================
// Memory Operations
// ============================================================================

// Safe memory copy (handles overlapping regions)
DAKT_FORCEINLINE void* memoryCopy(void* dest, const void* src, usize size) noexcept
{
    return std::memmove(dest, src, size);
}

// Fast memory copy (no overlap handling)
DAKT_FORCEINLINE void* memoryFastCopy(void* dest, const void* src, usize size) noexcept
{
    return std::memcpy(dest, src, size);
}

// Memory set
DAKT_FORCEINLINE void* memorySet(void* dest, int value, usize size) noexcept
{
    return std::memset(dest, value, size);
}

// Memory zero
DAKT_FORCEINLINE void* memoryZero(void* dest, usize size) noexcept
{
    return std::memset(dest, 0, size);
}

// Memory compare
[[nodiscard]] DAKT_FORCEINLINE int memoryCompare(const void* a, const void* b, usize size) noexcept
{
    return std::memcmp(a, b, size);
}

// Memory equal
[[nodiscard]] DAKT_FORCEINLINE bool memoryEqual(const void* a, const void* b, usize size) noexcept
{
    return std::memcmp(a, b, size) == 0;
}

// ============================================================================
// Allocator Concept
// ============================================================================

template <typename A>
concept Allocator = requires(A a, void* ptr, usize size, usize alignment) {
    { a.allocate(size, alignment) } -> std::same_as<void*>;
    { a.deallocate(ptr, size) } -> std::same_as<void>;
};

// ============================================================================
// Allocator Interface
// ============================================================================

class IAllocator
{
public:
    virtual ~IAllocator() = default;

    [[nodiscard]] virtual void* allocate(usize size, usize alignment = alignof(std::max_align_t)) = 0;
    virtual void deallocate(void* ptr, usize size) = 0;
    [[nodiscard]] virtual void* reallocate(void* ptr, usize oldSize, usize newSize,
                                           usize alignment = alignof(std::max_align_t)) = 0;

    // Typed allocation helpers
    template <typename T, typename... Args>
        requires std::constructible_from<T, Args...>
    [[nodiscard]] T* create(Args&&... args)
    {
        void* mem = allocate(sizeof(T), alignof(T));
        return std::construct_at(reinterpret_cast<T*>(mem), std::forward<Args>(args)...);
    }

    template <typename T>
    void destroy(T* ptr)
    {
        if (ptr)
        {
            std::destroy_at(ptr);
            deallocate(ptr, sizeof(T));
        }
    }

    template <typename T>
    [[nodiscard]] T* allocateArray(usize count)
    {
        return static_cast<T*>(allocate(sizeof(T) * count, alignof(T)));
    }

    template <typename T>
    void deallocateArray(T* ptr, usize count)
    {
        deallocate(ptr, sizeof(T) * count);
    }
};

// ============================================================================
// Default Heap Allocator
// ============================================================================

class HeapAllocator final : public IAllocator
{
public:
    [[nodiscard]] void* allocate(usize size, usize alignment = alignof(std::max_align_t)) override;
    void deallocate(void* ptr, usize size) override;
    [[nodiscard]] void* reallocate(void* ptr, usize oldSize, usize newSize,
                                   usize alignment = alignof(std::max_align_t)) override;

    [[nodiscard]] static HeapAllocator& instance();

private:
    HeapAllocator() = default;
};

// Global default allocator
[[nodiscard]] DAKT_API IAllocator& defaultAllocator();

// ============================================================================
// Arena Allocator (Linear/Bump Allocator)
// ============================================================================

class ArenaAllocator final : public IAllocator
{
public:
    explicit ArenaAllocator(usize capacity);
    ArenaAllocator(void* buffer, usize capacity);
    ~ArenaAllocator() override;

    DAKT_NON_COPYABLE_NON_MOVABLE(ArenaAllocator);

    [[nodiscard]] void* allocate(usize size, usize alignment = alignof(std::max_align_t)) override;
    void deallocate(void* ptr, usize size) override;  // No-op for arena
    [[nodiscard]] void* reallocate(void* ptr, usize oldSize, usize newSize,
                                   usize alignment = alignof(std::max_align_t)) override;

    // Reset arena (frees all allocations)
    void reset();

    // Get usage statistics
    [[nodiscard]] usize capacity() const noexcept { return m_capacity; }
    [[nodiscard]] usize used() const noexcept { return m_offset; }
    [[nodiscard]] usize remaining() const noexcept { return m_capacity - m_offset; }

private:
    byte* m_buffer;
    usize m_capacity;
    usize m_offset;
    bool m_ownsBuffer;
};

// ============================================================================
// Pool Allocator (Fixed-size block allocator)
// ============================================================================

class PoolAllocator final : public IAllocator
{
public:
    PoolAllocator(usize blockSize, usize blockCount);
    ~PoolAllocator() override;

    DAKT_NON_COPYABLE_NON_MOVABLE(PoolAllocator);

    [[nodiscard]] void* allocate(usize size, usize alignment = alignof(std::max_align_t)) override;
    void deallocate(void* ptr, usize size) override;
    [[nodiscard]] void* reallocate(void* ptr, usize oldSize, usize newSize,
                                   usize alignment = alignof(std::max_align_t)) override;

    // Pool info
    [[nodiscard]] usize blockSize() const noexcept { return m_blockSize; }
    [[nodiscard]] usize blockCount() const noexcept { return m_blockCount; }
    [[nodiscard]] usize freeCount() const noexcept { return m_freeCount; }

private:
    struct FreeBlock
    {
        FreeBlock* next;
    };

    byte* m_buffer;
    FreeBlock* m_freeList;
    usize m_blockSize;
    usize m_blockCount;
    usize m_freeCount;
};

// ============================================================================
// PMR Allocator Adapter (C++17 polymorphic allocator integration)
// ============================================================================

class PmrAllocatorAdapter : public std::pmr::memory_resource
{
public:
    explicit PmrAllocatorAdapter(IAllocator& allocator) : m_allocator(allocator) {}

protected:
    void* do_allocate(usize bytes, usize alignment) override { return m_allocator.allocate(bytes, alignment); }

    void do_deallocate(void* p, usize bytes, usize /*alignment*/) override { m_allocator.deallocate(p, bytes); }

    bool do_is_equal(const memory_resource& other) const noexcept override
    {
        auto* otherAdapter = dynamic_cast<const PmrAllocatorAdapter*>(&other);
        return otherAdapter && &m_allocator == &otherAdapter->m_allocator;
    }

private:
    IAllocator& m_allocator;
};

// ============================================================================
// Reference Counting
// ============================================================================

class RefCounted
{
public:
    RefCounted() : m_refCount(1) {}
    virtual ~RefCounted() = default;

    void addRef() noexcept { m_refCount.fetch_add(1, std::memory_order_relaxed); }

    void release() noexcept
    {
        if (m_refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            delete this;
        }
    }

    [[nodiscard]] u32 refCount() const noexcept { return m_refCount.load(std::memory_order_relaxed); }

protected:
    DAKT_NON_COPYABLE_NON_MOVABLE(RefCounted);

private:
    std::atomic<u32> m_refCount;
};

// ============================================================================
// Intrusive Reference Pointer
// ============================================================================

template <std::derived_from<RefCounted> T>
class Ref
{
public:
    constexpr Ref() noexcept = default;
    constexpr Ref(std::nullptr_t) noexcept {}

    explicit Ref(T* ptr, bool addRef = true) noexcept : m_ptr(ptr)
    {
        if (m_ptr && addRef)
        {
            m_ptr->addRef();
        }
    }

    Ref(const Ref& other) noexcept : m_ptr(other.m_ptr)
    {
        if (m_ptr)
        {
            m_ptr->addRef();
        }
    }

    Ref(Ref&& other) noexcept : m_ptr(other.m_ptr) { other.m_ptr = nullptr; }

    template <std::derived_from<T> U>
    Ref(const Ref<U>& other) noexcept : m_ptr(other.get())
    {
        if (m_ptr)
        {
            m_ptr->addRef();
        }
    }

    ~Ref()
    {
        if (m_ptr)
        {
            m_ptr->release();
        }
    }

    Ref& operator=(const Ref& other) noexcept
    {
        if (this != &other)
        {
            if (m_ptr)
                m_ptr->release();
            m_ptr = other.m_ptr;
            if (m_ptr)
                m_ptr->addRef();
        }
        return *this;
    }

    Ref& operator=(Ref&& other) noexcept
    {
        if (this != &other)
        {
            if (m_ptr)
                m_ptr->release();
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }

    Ref& operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    void reset() noexcept
    {
        if (m_ptr)
        {
            m_ptr->release();
            m_ptr = nullptr;
        }
    }

    void reset(T* ptr, bool addRef = true) noexcept
    {
        if (m_ptr)
            m_ptr->release();
        m_ptr = ptr;
        if (m_ptr && addRef)
            m_ptr->addRef();
    }

    [[nodiscard]] T* release() noexcept
    {
        T* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    [[nodiscard]] T* get() const noexcept { return m_ptr; }
    [[nodiscard]] T& operator*() const noexcept { return *m_ptr; }
    [[nodiscard]] T* operator->() const noexcept { return m_ptr; }

    [[nodiscard]] explicit operator bool() const noexcept { return m_ptr != nullptr; }

    auto operator<=>(const Ref&) const = default;
    bool operator==(std::nullptr_t) const noexcept { return m_ptr == nullptr; }

private:
    T* m_ptr = nullptr;

    template <std::derived_from<RefCounted> U>
    friend class Ref;
};

template <std::derived_from<RefCounted> T, typename... Args>
    requires std::constructible_from<T, Args...>
[[nodiscard]] Ref<T> makeRef(Args&&... args)
{
    return Ref<T>(new T(std::forward<Args>(args)...), false);
}

// ============================================================================
// Smart Pointer Aliases
// ============================================================================

template <typename T, typename Deleter = std::default_delete<T>>
using Unique = std::unique_ptr<T, Deleter>;

template <typename T, typename... Args>
    requires std::constructible_from<T, Args...>
[[nodiscard]] Unique<T> makeUnique(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using Shared = std::shared_ptr<T>;

template <typename T, typename... Args>
    requires std::constructible_from<T, Args...>
[[nodiscard]] Shared<T> makeShared(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
using Weak = std::weak_ptr<T>;

// ============================================================================
// Memory Debugging
// ============================================================================

#if defined(DAKT_DEBUG)
struct MemoryStats
{
    std::atomic<i64> totalAllocations{0};
    std::atomic<i64> totalDeallocations{0};
    std::atomic<i64> currentAllocations{0};
    std::atomic<i64> totalBytesAllocated{0};
    std::atomic<i64> currentBytesAllocated{0};
    std::atomic<i64> peakBytesAllocated{0};

    void recordAllocation(usize bytes)
    {
        totalAllocations.fetch_add(1, std::memory_order_relaxed);
        currentAllocations.fetch_add(1, std::memory_order_relaxed);
        totalBytesAllocated.fetch_add(static_cast<i64>(bytes), std::memory_order_relaxed);
        i64 current = currentBytesAllocated.fetch_add(static_cast<i64>(bytes), std::memory_order_relaxed) +
                      static_cast<i64>(bytes);

        // Update peak
        i64 peak = peakBytesAllocated.load(std::memory_order_relaxed);
        while (current > peak && !peakBytesAllocated.compare_exchange_weak(peak, current, std::memory_order_relaxed))
        {
        }
    }

    void recordDeallocation(usize bytes)
    {
        totalDeallocations.fetch_add(1, std::memory_order_relaxed);
        currentAllocations.fetch_sub(1, std::memory_order_relaxed);
        currentBytesAllocated.fetch_sub(static_cast<i64>(bytes), std::memory_order_relaxed);
    }
};

[[nodiscard]] DAKT_API MemoryStats& getMemoryStats();
#endif

}  // namespace dakt::core