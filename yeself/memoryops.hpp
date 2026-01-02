#ifndef YESELF_MEMORYOPS_HPP
#define YESELF_MEMORYOPS_HPP

#include <stdint.h>
#include <stddef.h>


template<typename T>
inline void copy_memory_tml(T* dst, const T* src, size_t count) 
{
    if constexpr(sizeof(T) == 1) 
    {
        asm volatile(
            "cld; rep movsb"
            : "+D"(dst), "+S"(src), "+c"(count)
            : : "memory"
        );
    }
    else if constexpr(sizeof(T) == 2) 
    {
        asm volatile(
            "cld; rep movsw"
            : "+D"(dst), "+S"(src), "+c"(count)
            : : "memory"
        );
    }
    else if constexpr(sizeof(T) == 4) 
    {
        asm volatile(
            "cld; rep movsd"
            : "+D"(dst), "+S"(src), "+c"(count)
            : : "memory"
        );
    }
    else 
    {
        for(size_t i{}; i < count; ++i)
            dst[i] = src[i];
    }
}


template<typename T>
inline void fill_memory_tml(T* dst, T value, size_t count) 
{
    if constexpr(sizeof(T) == 1) 
    {
        asm volatile(
            "cld; rep stosb"
            : "+D"(dst), "+c"(count)
            : "a"(static_cast<uint8_t>(value))
            : "memory"
        );
    }
    else if constexpr(sizeof(T) == 2) 
    {
        asm volatile(
            "cld; rep stosw"
            : "+D"(dst), "+c"(count)
            : "a"(static_cast<uint16_t>(value))
            : "memory"
        );
    }
    else if constexpr(sizeof(T) == 4) 
    {
        asm volatile(
            "cld; rep stosd"
            : "+D"(dst), "+c"(count)
            : "a"(static_cast<uint32_t>(value))
            : "memory"
        );
    }
    else 
    {
        for(size_t i{}; i < count; ++i)
            dst[i] = value;
    }
}


#endif // YESELF_MEMORYOPS_HPP
