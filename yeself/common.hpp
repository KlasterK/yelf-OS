#ifndef YESELF_COMMON_HPP
#define YESELF_COMMON_HPP

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

#endif // YESELF_COMMON_HPP
