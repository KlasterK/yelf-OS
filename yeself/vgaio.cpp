#include "vgaio.hpp"

using namespace VGA;

Pair<volatile Char *> TerminalFile::get_page_begin_end(int page_number)
{
    auto begin = VideoMemory() + page_number * Width * Height;
    return {begin, begin + Width * Height};
}

int TerminalFile::write(const void *buf, size_t n)
{
    auto char_buf = static_cast<const char *>(buf);
    auto [begin, end] = get_page_begin_end(0);
    size_t i{};
    char c{};

    for(; i < n; ++i)
    {
        c = char_buf[i];
        if(c == '\b')
        {
            m_cursor = max(begin, m_cursor - 1);
        }
        else if(c == '\r')
        {
            m_cursor -= (m_cursor - begin) % Width;
        }
        else if(c == '\n')
        {
            m_cursor -= (m_cursor - begin) % Width;
            m_cursor += Width;

            if(m_cursor >= end)
                m_cursor = begin;
        }
        else if(c == '\t')
        {
            m_cursor += 8 - (m_cursor - begin) % 8;
            if(m_cursor >= end)
                m_cursor = begin;
        }
        else
        {
            *(Char *)m_cursor++ = {.text=c, .style=m_style};
            if(m_cursor >= end)
                m_cursor = begin;
        }
    }

    return i;
}

int VGA::TerminalFile::seek(int offset, SeekFrom whence)
{
    auto [begin, end] = get_page_begin_end(0);

    if(whence == SeekFrom::Begin)
        m_cursor = begin + max(0, offset);
    else if(whence == SeekFrom::CurrentPosition)
        m_cursor = clamp(m_cursor + offset, begin, end-1);
    else if(whence == SeekFrom::End)
        m_cursor = end - max(0, offset) - 1;

    return m_cursor - begin;
}
