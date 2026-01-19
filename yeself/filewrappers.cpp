#include "filewrappers.hpp"
#include "common.hpp"
#include "stl.hpp"

BufferFile::BufferFile(void *begin, void *end, bool readable, bool writable)
    : m_begin((char *)begin)
    , m_end((char *)end)
    , m_cursor((char *)begin)
    , m_readable(readable)
    , m_writable(writable)
{}

int BufferFile::read(void *buf, size_t n)
{
    if(!m_readable)
        return -1;
    
    if(m_cursor >= m_end)
        return 0;
    
    auto char_buf = (char *)buf;
    size_t i{};
    while(i < n)
    {
        char_buf[i++] = *m_cursor++;
        if(m_cursor >= m_end)
            break;
    }
    return i;
}

int BufferFile::write(const void *buf, size_t n)
{
    if(!m_writable)
        return -1;
    
    if(m_cursor >= m_end)
        return 0;
    
    auto char_buf = (const char *)buf;
    size_t i{};
    while(i < n)
    {
        *m_cursor++ = char_buf[i++];
        if(m_cursor >= m_end)
            break;
    }
    return i;
}

int BufferFile::seek(int offset, SeekFrom whence)
{
    if (whence == SeekFrom::Begin)
        m_cursor = m_begin + max(0, offset);
    else if (whence == SeekFrom::CurrentPosition)
        m_cursor = clamp(m_cursor + offset, m_begin, m_end - 1);
    else if (whence == SeekFrom::End)
        m_cursor = m_end - max(0, offset) - 1;

    return m_cursor - m_begin;
}
