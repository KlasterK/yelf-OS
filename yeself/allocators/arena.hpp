#pragma once

#include "allocators/rallocator.hpp"

class ArenaAllocator
{
public:
    static constexpr size_t MaxAllocationSize{SIZE_MAX}, MaxAlignment{1zu << (SIZE_WIDTH - 1)};

public:
    ArenaAllocator(uint8_t *buffer, size_t buffer_size) 
        : m_current(buffer)
        , m_end(buffer + buffer_size)
    {}

    void *allocate(size_t size, size_t alignment) 
    {
        if(m_current == nullptr)
            return nullptr;

        auto current_i = reinterpret_cast<uintptr_t>(m_current);
        auto aligned_current_i = ceildiv(current_i, alignment);
        auto *aligned_current_p = reinterpret_cast<uint8_t *>(aligned_current_i);

        if(aligned_current_p + size >= m_end)
            return nullptr;

        m_current = aligned_current_p + size;

        return aligned_current_p;
    }

    void free(void *) {}
    bool expand(void *, size_t) { return false; }
    bool shrink(void *, size_t) { return false; }

private:
    uint8_t *m_current{}, *m_end{};
};

static_assert(RAllocator<ArenaAllocator>);
