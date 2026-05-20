#pragma once

#include "common.hpp"
#include "stl.hpp"

template <typename T>
concept RAllocator = requires(T allocator, void *ptr, size_t size, uint8_t align_exponent, size_t delta_size)
{
    { allocator.allocate(size, align_exponent) } -> SameAs<void *>;
    { allocator.free(ptr) } -> SameAs<void>;
    { allocator.expand(ptr, delta_size) } -> SameAs<bool>;
    { allocator.shrink(ptr, delta_size) } -> SameAs<bool>;

    { T::MaxAllocationSize } -> SameAs<const size_t &>;
    requires (T::MaxAllocationSize == T::MaxAllocationSize); // requires it to be const expression
    { T::MaxAlignExponent } -> SameAs<const uint8_t &>;
    requires (T::MaxAlignExponent == T::MaxAlignExponent);   // requires it to be const expression
};
