#ifndef YESELF_COMMON_HPP
#define YESELF_COMMON_HPP

#include <stdint.h>
#include <stddef.h>

#ifndef __cdecl
    #define __cdecl __attribute__((cdecl))
#endif

#ifndef __packed
    #define __packed __attribute__((packed))
#endif

// C++ export
extern "C" void __cdecl c_main();

template<typename T>
struct Pair
{
    T first, second;
};

template<typename T>
constexpr T min(T a, T b) { return (a < b) ? a : b; }

template<typename T>
constexpr T max(T a, T b) { return (a > b) ? a : b; }

template<typename T>
constexpr T clamp(T x, T lower, T upper) { return max(lower, min(upper, x)); }

constexpr size_t strlen(const char* s) 
{
    size_t i{};
    while(s[i] != 0) ++i;
    return i;
}

#endif // YESELF_COMMON_HPP
