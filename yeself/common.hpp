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

inline void *operator new(size_t, void *p) { return p; }

#endif // YESELF_COMMON_HPP
