#ifndef YESELF_IFILE_HPP
#define YESELF_IFILE_HPP

#include <stddef.h>

class IFile
{
public:
    enum class SeekFrom {Begin, CurrentPosition, End};

public:
    virtual ~IFile() = default;
    virtual int read(void *buf, size_t n) = 0;
    virtual int write(const void *buf, size_t n) = 0;
    virtual int seek(int offset, SeekFrom whence) = 0;
    virtual int close() = 0;
};

#endif // YESELF_IFILE_HPP
