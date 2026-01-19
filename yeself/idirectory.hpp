#ifndef YESELF_IDIRECTORY_HPP
#define YESELF_IDIRECTORY_HPP

#include "ifile.hpp"

class IDirectory
{
public:
    enum class NodeType : uint8_t {None=0, File=1, Directory=2};
    using Iterator = int32_t;
    static constexpr Iterator before_begin_iter() { return -1; }
    static constexpr Iterator end_iter() { return -2; }
    static constexpr bool is_error_iter(Iterator it) { return it < 0; }

public:
    virtual ~IDirectory() = default;

    virtual Iterator next(Iterator prev_it) const = 0;
    virtual Iterator find(const char *name) const = 0;

    virtual Iterator create_file(const char *name) = 0;
    virtual Iterator create_directory(const char *name) = 0;
    virtual bool remove(Iterator it) = 0;

    virtual size_t get_name_length(Iterator it) const = 0;
    virtual size_t get_name(Iterator it, char *buf, size_t size) const = 0;
    virtual NodeType get_node_type(Iterator it) const = 0;

    virtual size_t get_node_object_size(Iterator it) = 0;
    virtual void *open(Iterator it, /* alignas 16 */ void *buf) = 0;
};

#endif // YESELF_IDIRECTORY_HPP
