#ifndef YESELF_TYPEDALLOCATOR_HPP
#define YESELF_TYPEDALLOCATOR_HPP

#include "common.hpp"
#include "stl.hpp"

template<typename T>
class BitmapAllocator
{
public:
    BitmapAllocator(T *obj_buffer, size_t capacity, uint32_t *bitmaps)
        : m_buf(obj_buffer)
        , m_size(capacity)
        , m_bitmaps(bitmaps)
    {

    }

    template<typename... Args>
    T *make_alloc(Args... args)
    {
        for(size_t i{}; i < ceildiv(m_size, 32); ++i)
        {
            uint32_t &target_int = m_bitmaps[int_offset];
            if(target_int == 0xFFFFFFFF)
                continue;
            
            for(size_t bit{}; bit < 32; ++bit)
            {
                if(target_int & (1 << bit) == 0)
                {
                    target_int |= (1 << bit);
                    size_t offset = target_int * 32 + bit;
                    T *ptr = m_buf + offset;
                    ::new (ptr) T(Forward<Args>...);
                    return ptr;
                }
            }
        }
        return nullptr;
    }

    bool free(T *ptr)
    {
        size_t offset = ptr - m_buf;
        size_t int_offset = offset / 32;
        size_t bit_offset = offset % 32;
        uint32_t &target_int = m_bitmaps[int_offset];
        target_int &= ~(1 << bit_offset);
    }

private:
    T *m_buf;
    size_t m_size;
    uint32_t *m_bitmaps;
};

#endif // YESELF_TYPEDALLOCATOR_HPP
