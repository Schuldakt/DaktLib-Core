// ============================================================================
// DaktLib Core - Memory Implementation (C++23)
// ============================================================================

#include "dakt/core/Memory.hpp"

#include <cstdlib>
#include <memory>
#include <new>

namespace dakt::core
{

// ============================================================================
// Heap Allocator
// ============================================================================

void* HeapAllocator::allocate(usize size, usize alignment)
{
    if (size == 0)
        return nullptr;

#if defined(DAKT_PLATFORM_WINDOWS)
    void* ptr = _aligned_malloc(size, alignment);
#else
    void* ptr = std::aligned_alloc(alignment, alignUp(size, alignment));
#endif

#if defined(DAKT_DEBUG)
    if (ptr)
    {
        getMemoryStats().recordAllocation(size);
    }
#endif

    return ptr;
}

void HeapAllocator::deallocate(void* ptr, usize size)
{
    if (!ptr)
        return;

#if defined(DAKT_DEBUG)
    getMemoryStats().recordDeallocation(size);
#else
    DAKT_UNUSED(size);
#endif

#if defined(DAKT_PLATFORM_WINDOWS)
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}

void* HeapAllocator::reallocate(void* ptr, usize oldSize, usize newSize, usize alignment)
{
    if (newSize == 0)
    {
        deallocate(ptr, oldSize);
        return nullptr;
    }

    if (!ptr)
    {
        return allocate(newSize, alignment);
    }

    // Allocate new, copy, free old
    void* newPtr = allocate(newSize, alignment);
    if (newPtr)
    {
        std::memcpy(newPtr, ptr, std::min(oldSize, newSize));
        deallocate(ptr, oldSize);
    }

    return newPtr;
}

HeapAllocator& HeapAllocator::instance()
{
    static HeapAllocator instance;
    return instance;
}

IAllocator& defaultAllocator()
{
    return HeapAllocator::instance();
}

// ============================================================================
// Arena Allocator
// ============================================================================

ArenaAllocator::ArenaAllocator(usize capacity)
    : m_buffer(static_cast<byte*>(std::malloc(capacity))), m_capacity(capacity), m_offset(0), m_ownsBuffer(true)
{
    DAKT_ASSERT_MSG(m_buffer != nullptr, "Failed to allocate arena buffer");
}

ArenaAllocator::ArenaAllocator(void* buffer, usize capacity)
    : m_buffer(static_cast<byte*>(buffer)), m_capacity(capacity), m_offset(0), m_ownsBuffer(false)
{
    DAKT_ASSERT_MSG(m_buffer != nullptr, "Arena buffer cannot be null");
}

ArenaAllocator::~ArenaAllocator()
{
    if (m_ownsBuffer && m_buffer)
    {
        std::free(m_buffer);
    }
}

void* ArenaAllocator::allocate(usize size, usize alignment)
{
    if (size == 0)
        return nullptr;

    usize alignedOffset = alignUp(m_offset, alignment);

    if (alignedOffset + size > m_capacity)
    {
        return nullptr;  // Out of memory
    }

    void* ptr = m_buffer + alignedOffset;
    m_offset = alignedOffset + size;
    return ptr;
}

void ArenaAllocator::deallocate(void* ptr, usize size)
{
    // No-op for arena allocator
    DAKT_UNUSED(ptr);
    DAKT_UNUSED(size);
}

void* ArenaAllocator::reallocate(void* ptr, usize oldSize, usize newSize, usize alignment)
{
    // Simple strategy: allocate new and copy
    if (newSize <= oldSize)
    {
        return ptr;
    }

    void* newPtr = allocate(newSize, alignment);
    if (newPtr && ptr)
    {
        std::memcpy(newPtr, ptr, oldSize);
    }
    return newPtr;
}

void ArenaAllocator::reset()
{
    m_offset = 0;
}

// ============================================================================
// Pool Allocator
// ============================================================================

PoolAllocator::PoolAllocator(usize blockSize, usize blockCount)
    : m_buffer(nullptr), m_freeList(nullptr), m_blockSize(std::max(blockSize, sizeof(FreeBlock))),
      m_blockCount(blockCount), m_freeCount(blockCount)
{
    m_buffer = static_cast<byte*>(std::malloc(m_blockSize * m_blockCount));
    DAKT_ASSERT_MSG(m_buffer != nullptr, "Failed to allocate pool buffer");

    // Initialize free list
    for (usize i = 0; i < m_blockCount; ++i)
    {
        auto* block = std::launder(reinterpret_cast<FreeBlock*>(m_buffer + i * m_blockSize));
        block->next = m_freeList;
        m_freeList = block;
    }
}

PoolAllocator::~PoolAllocator()
{
    if (m_buffer)
    {
        std::free(m_buffer);
    }
}

void* PoolAllocator::allocate(usize size, usize alignment)
{
    DAKT_UNUSED(alignment);

    if (size > m_blockSize || !m_freeList)
    {
        return nullptr;
    }

    FreeBlock* block = m_freeList;
    m_freeList = block->next;
    --m_freeCount;

    return block;
}

void PoolAllocator::deallocate(void* ptr, usize size)
{
    DAKT_UNUSED(size);

    if (!ptr)
        return;

    // Verify pointer is within our buffer
    DAKT_ASSERT(ptr >= m_buffer && ptr < m_buffer + m_blockSize * m_blockCount);

    auto* block = static_cast<FreeBlock*>(ptr);
    block->next = m_freeList;
    m_freeList = block;
    ++m_freeCount;
}

void* PoolAllocator::reallocate(void* ptr, usize oldSize, usize newSize, usize alignment)
{
    DAKT_UNUSED(oldSize);
    DAKT_UNUSED(alignment);

    if (newSize > m_blockSize)
    {
        return nullptr;
    }
    return ptr;  // Block size is fixed
}

// ============================================================================
// Memory Stats
// ============================================================================

#if defined(DAKT_DEBUG)
MemoryStats& getMemoryStats()
{
    static MemoryStats stats;
    return stats;
}
#endif

}  // namespace dakt::core