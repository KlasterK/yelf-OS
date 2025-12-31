#include "vgaio.hpp"

using namespace VGA;

constexpr uint16_t _vga_port_register = 0x3D4;
constexpr uint16_t _vga_port_value = 0x3D5;
constexpr uint8_t _vga_start_addr_upper_byte = 0x0C;
constexpr uint8_t _vga_start_addr_lower_byte = 0x0D;

void VGA::TerminalFile::scroll_up(int nlines)
{
    uint16_t start_address = (m_top_line_number + nlines) * Width;

    asm volatile("out dx, al" : : "a"(_vga_start_addr_upper_byte), "d"(_vga_port_register));
    asm volatile("out dx, al" : : "a"((start_address >> 8) & 0xFF), "d"(_vga_port_value));

    asm volatile("out dx, al" : : "a"(_vga_start_addr_lower_byte), "d"(_vga_port_register));
    asm volatile("out dx, al" : : "a"(start_address & 0xFF), "d"(_vga_port_value));
}

int TerminalFile::write(const void *buf, size_t n)
{
    auto char_buf = static_cast<const char *>(buf);
    size_t i{};
    char c{};

    for(; i < n; ++i)
    {
        c = char_buf[i];
        if(c == '\b')
        {
            m_cursor = max(PageBegin(), m_cursor - 1);
        }
        else if(c == '\r')
        {
            m_cursor -= (m_cursor - PageBegin()) % Width;
        }
        else if(c == '\n')
        {
            m_cursor -= (m_cursor - PageBegin()) % Width;
            m_cursor += Width;

            if(m_cursor >= PageEnd())
                scroll_up();
        }
        else if(c == '\t')
        {
            m_cursor += 8 - (m_cursor - PageBegin()) % 8;
            if(m_cursor >= PageEnd())
                scroll_up();
        }
        else if(c == '\f')
        {
            m_top_line_number = 0;
            scroll_up(0);
            m_cursor = PageBegin();
        }
        else
        {
            *(Char *)m_cursor++ = {.text=c, .style=m_style};
            if(m_cursor >= PageEnd())
                scroll_up();
        }
    }

    return i;
}

int TerminalFile::seek(int offset, SeekFrom whence)
{
    if(whence == SeekFrom::Begin)
        m_cursor = PageBegin() + max(0, offset);
    else if(whence == SeekFrom::CurrentPosition)
        m_cursor = clamp(m_cursor + offset, PageBegin(), PageEnd() - 1);
    else if(whence == SeekFrom::End)
        m_cursor = PageEnd() - max(0, offset) - 1;

    return m_cursor - PageBegin();
}
