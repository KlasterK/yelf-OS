#ifndef YESELF_STRINGOPS_HPP
#define YESELF_STRINGOPS_HPP

#include "common.hpp"
#include "memoryops.hpp"

constexpr size_t strlen(const char *s) 
{
    size_t i{};
    while(s[i] != 0)
        ++i;
    return i;
}

constexpr int strcmp(const char *a, const char *b)
{
    while(*a && *a++ == *b++) {}
    return *(const unsigned char *)--a - *(const unsigned char *)--b;
}

constexpr int strncmp(const char *a, const char *b, size_t n)
{
    while(n-- && *a != 0 && *b != 0)
    {
        if(*a++ != *b++)
            return *(const unsigned char *)--a - *(const unsigned char *)--b;
    }
    return n == 0 ? 0 : *a - *b;
}

constexpr size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t src_len = strlen(src);
    if (size == 0)
        return src_len;

    size_t copy_size = (src_len < size - 1) ? src_len : size - 1;
    
    copy_memory_tml(dst, src, copy_size);
    dst[copy_size] = '\0';

    return src_len;
}

constexpr bool is_whitespace(char c) 
{ 
    return c == ' '  || c == '\t' || c == '\n' || c == '\v'
        || c == '\f' || c == '\r';
}

constexpr const char *string_lstrip(const char *s, size_t n)
{
    for(size_t i{}; i < n; ++i)
    {
        if(!is_whitespace(s[i]))
            return s + i;
    }
    return s;
}

constexpr char *string_lstrip(char *s)
{
    for(size_t i{}; s[i] != 0; ++i)
    {
        if(!is_whitespace(s[i]))
            return s + i;
    }
    return s;
}

constexpr size_t string_rstrip(char *s)
{
    if(*s == 0)
        return 0;

    char *end = s + strlen(s) - 1;
    while(end >= s && is_whitespace(*end--)) {}
    end += 2;
    *end = 0;
    return end - s;
}

constexpr Pair<char *, size_t> string_strip(char *s)
{
    char *right_s = string_lstrip(s);
    size_t nstripped = string_rstrip(right_s);
    return {right_s, nstripped};
}

#endif // YESELF_STRINGOPS_HPP
