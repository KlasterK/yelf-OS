#ifndef YESELF_IFILE_HPP
#define YESELF_IFILE_HPP

#include <stdint.h>
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
    virtual int ioctl(uint32_t, void *) { return -1; };
};

namespace IOCtlFunctions
{
    constexpr uint32_t IsTerminal = 128;
    constexpr uint32_t GetTerminalWidth = 129;
    constexpr uint32_t GetTerminalHeight = 130;
    constexpr uint32_t GetLineCounter = 131;
}

#endif // YESELF_IFILE_HPP
