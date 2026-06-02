#pragma once

#include "common.hpp"
#include "stl.hpp"

/// @brief Memory allocator concept.
template <typename T>
concept RAllocator = requires(T allocator, void *ptr, size_t size, size_t align, size_t delta_size)
{
    /// @brief Allocates a memory block.
    ///
    /// @param size     Size in bytes to allocate. Must be less than MaxAllocationSize.
    /// @param align    Minimally required alignment of the block, must be a power of 2.
    ///
    /// @returns Pointer to the allocated memory, nullptr if the allocation failed.
    ///
    /// @note Shouldn't panic if an OOM has occured.
    { allocator.allocate(size, align) } -> SameAs<void *>;

    /// @brief Frees a memory block allocated in this allocator.
    ///
    /// @param ptr      Pointer to the memory block.
    ///
    /// @note Should panic if the passed pointer is nullptr, doesn't belong to this allocator
    ///       or already freed.
    { allocator.free(ptr) } -> SameAs<void>;

    /// @brief Tries to expand a memory block already allocated in this allocator.
    ///
    /// The implementation should look if there's free space after the memory block.
    /// If it's enough to expand the block, then the block is expanded and true is returned.
    /// If it's not, then false is returned and the block is unchanged.
    ///
    /// @param ptr          Pointer to the memory block.
    /// @param delta_size   Count of bytes by which the memory block should be expanded.
    ///
    /// @returns True if succeeded, false if expanding is inapplicable.
    ///
    /// @note Should panic if the passed pointer is nullptr, doesn't belong to this allocator
    ///       or already freed.
    { allocator.expand(ptr, delta_size) } -> SameAs<bool>;

    /// @brief Tries to shrink a memory block already allocated in this allocator.
    ///
    /// If the implementation can mark the space after the block as free for further
    /// allocations, then it should reduce the block size, mark the space as free
    /// and return true. If it can't (e.g. it's an arena and the memory is in the middle)
    /// then it does nothing and returns false.
    ///
    /// @param ptr          Pointer to the memory block.
    /// @param delta_size   Count of bytes by which the memory block should be shrunk.
    ///
    /// @returns True if succeeded, false if shrinking is inapplicable.
    ///
    /// @note Should panic if the passed pointer is nullptr, doesn't belong to this allocator
    ///       or already freed.
    ///
    /// @note Passing delta_size >= the block size won't free the block. Instead, it panics.
    { allocator.shrink(ptr, delta_size) } -> SameAs<bool>;

    /// @note Allocating/expanding a block with a size bigger than this value shouldn't lead to a panic,
    ///       instead the implementation should return nullptr/false.
    { T::MaxAllocationSize } -> SameAs<const size_t &>;

    /// @note Allocating a block with an alignment bigger than this value shouldn't lead to a panic,
    ///       instead the implementation should return nullptr.
    { T::MaxAlignment } -> SameAs<const size_t &>;

    // Constant expression check
    requires (T::MaxAllocationSize == T::MaxAllocationSize);
    requires (T::MaxAlignment == T::MaxAlignment);
};
