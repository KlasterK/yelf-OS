#ifndef YESELF_FILEWRAPPERS_HPP
#define YESELF_FILEWRAPPERS_HPP

#include "ifile.hpp"

class ComposedTTY : public IFile
{
public:
    inline ComposedTTY(IFile &out, IFile &in) : m_out(out), m_in(in) {}
    inline int read(void *buf, size_t n) override { return m_in.read(buf, n); };
    inline int write(const void *buf, size_t n) override { return m_out.write(buf, n); };
    inline int seek(int offset, SeekFrom whence) override { return m_out.seek(offset, whence); };
    inline int close() override { return 0; };
    inline int ioctl(uint32_t function, void *argument) override 
    { 
        int ret = m_out.ioctl(function, argument);
        if(ret >= 0)
            return ret;
        return m_in.ioctl(function, argument);
    }

private:
    IFile &m_out, &m_in;
};

class BufferFile : public IFile
{
public:
    BufferFile(void *begin, void *end, bool readable, bool writable);
    int read(void *buf, size_t n) override;
    int write(const void *buf, size_t n) override;
    int seek(int offset, SeekFrom whence) override;
    inline int close() override { return 0; };

private:
    char *m_begin{}, *m_end{}, *m_cursor{};
    bool m_readable{}, m_writable{};
};

#endif // YESELF_FILEWRAPPERS_HPP
