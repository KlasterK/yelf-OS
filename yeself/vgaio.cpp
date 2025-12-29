#include "vgaio.hpp"

using namespace VGA;

Pair<volatile Char *> Terminal::get_page_begin_end(int page_number)
{
    auto begin = VideoMemory() + page_number * Width * Height;
    return {begin, begin + Width * Height};
}

void Terminal::putc(char c)
{
    auto [begin, end] = get_page_begin_end(0);

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
        *(Char *)m_cursor = {.text=c, .style=m_style};
        ++m_cursor;
        if(m_cursor >= end)
            m_cursor = begin;
    }
}

void Terminal::puts(const char *string)
{
    while(*string != 0)
        putc(*string++);
}

void Terminal::write(const char *data, size_t n)
{
    for(auto end = data + n; data != end; ++data)
        putc(*data);
}

void VGA::Terminal::seek(unsigned x, unsigned y)
{
    m_cursor = get_page_begin_end(0).first 
             + min(y, Height-1) * Width
             + min(x, Width-1);
}

Pair<unsigned> VGA::Terminal::tell()
{
    unsigned offset = m_cursor - get_page_begin_end(0).first;
    return {offset % Width, offset / Width};
}
