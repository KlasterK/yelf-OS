#ifndef YESELF_MEMORYOPS_HPP
#define YESELF_MEMORYOPS_HPP

#include <stdint.h>
#include <stddef.h>
#include "stl.hpp"


template<typename T>
inline void copy_memory_tml(T* dst, const T* src, size_t count) 
{
    size_t nbytes = sizeof(T) * count;
    if(nbytes % 4 == 0)
    {
        size_t ndwords = nbytes / 4;
        asm volatile(
            "cld; rep movsd"
            : "+D"(dst), "+S"(src), "+c"(ndwords)
            : : "memory"
        );
    }
    else if(nbytes == 2) 
    {
        size_t nwords = nbytes / 2;
        asm volatile(
            "cld; rep movsw"
            : "+D"(dst), "+S"(src), "+c"(nwords)
            : : "memory"
        );
    }
    else 
    {
        asm volatile(
            "cld; rep movsb"
            : "+D"(dst), "+S"(src), "+c"(nbytes)
            : : "memory"
        );
    }
}


template<typename T>
inline void fill_memory_tml(T* dst, T value, size_t count) 
{
    size_t nbytes = sizeof(T) * count;
    if(nbytes % 4 == 0) 
    {
        size_t ndwords = nbytes / 4;
        asm volatile(
            "cld; rep stosd"
            : "+D"(dst), "+c"(ndwords)
            : "a"(static_cast<uint32_t>(value))
            : "memory"
        );
    }
    else if(nbytes % 2 == 0) 
    {
        size_t nwords = nbytes / 2;
        asm volatile(
            "cld; rep stosw"
            : "+D"(dst), "+c"(nwords)
            : "a"(static_cast<uint16_t>(value))
            : "memory"
        );
    }
    else
    {
        asm volatile(
            "cld; rep stosb"
            : "+D"(dst), "+c"(nbytes)
            : "a"(static_cast<uint8_t>(value))
            : "memory"
        );
    }
}


template<typename T>
inline bool cmp_memory_tml(const T *a, const T *b, size_t count)
{
    size_t nbytes = sizeof(T) * count;
    uint8_t success{};
    if(nbytes % 4 == 0) 
    {
        size_t ndwords = nbytes / 4;
        asm volatile(
            "cld; repe cmpsd; setz al"
            : "+D"(a), "+S"(b), "+c"(ndwords), "=a"(success) 
            : :
        );
    }
    else if(nbytes % 2 == 0) 
    {
        size_t nwords = nbytes / 2;
        asm volatile(
            "cld; repe cmpsw; setz al"
            : "+D"(a), "+S"(b), "+c"(nwords), "=a"(success) 
            : :
        );
    }
    else
    {
        asm volatile(
            "cld; repe cmpsb; setz al"
            : "+D"(a), "+S"(b), "+c"(nbytes), "=a"(success) 
            : :
        );
    }
    return success;
}


#endif // YESELF_MEMORYOPS_HPP
