#ifndef YESELF_VGAIO_HPP
#define YESELF_VGAIO_HPP

#include "common.hpp"
#include <stdint.h>
#include <stddef.h>

namespace VGA 
{

union CharStyle
{
    static constexpr uint8_t Black      = 0b000;
    static constexpr uint8_t Red        = 0b100;
    static constexpr uint8_t Yellow     = 0b110;
    static constexpr uint8_t Green      = 0b010;
    static constexpr uint8_t Cyan       = 0b011;
    static constexpr uint8_t Blue       = 0b001;
    static constexpr uint8_t Magenta    = 0b101;
    static constexpr uint8_t White      = 0b111;

    uint8_t raw = 0b00000111;
    struct
    {
        uint8_t fg_rgb       : 3;
        uint8_t fg_intensity : 1;
        uint8_t bg_rgb       : 3;
        uint8_t bg_intensity : 1;
    } __packed;
    struct
    {
        uint8_t fg_irgb : 4;
        uint8_t bg_irgb : 4;
    } __packed;
} __packed;
static_assert(sizeof(CharStyle) == 1);

union Char
{
    uint16_t raw;
    struct
    {
        char text = ' ';
        CharStyle style;
    } __packed;
} __packed;
static_assert(sizeof(Char) == 2);

class Terminal
{
public:
    static constexpr unsigned Width = 80, Height = 25;

    void putc(char c);
    void puts(const char *string);
    void write(const char *data, size_t n);
    void printf(const char *fmt, ...);

    void seek(unsigned x, unsigned y);
    Pair<unsigned> tell();

    inline CharStyle get_style() { return m_style; }
    inline void set_style(CharStyle style) { m_style = style; }

private:
    static inline volatile Char *VideoMemory() { return (volatile Char *)0xB8000; }

    static Pair<volatile Char *> get_page_begin_end(int page_number);

    CharStyle m_style;
    volatile Char *m_cursor = VideoMemory();
};

} // namespace VGA

#endif // YESELF_VGAIO_HPP
