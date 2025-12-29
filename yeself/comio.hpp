#ifndef YESELF_COMIO_HPP
#define YESELF_COMIO_HPP

#include <stdint.h>
#include <stddef.h>

inline void com_putc(char c);
void com_printf(const char *fmt, ...);
inline void com_write(const char *data, size_t n);

inline char com_getc();

// Implementation

constexpr int _com_port_data = 0x3F8;
constexpr int _com_port_lsr  = _com_port_data + 5;

inline void com_putc(char c)
{
    for(uint8_t status{};;)
    {
        asm volatile("in al, dx" : "=a"(status) : "d"(_com_port_lsr));
        if(status & 0x20) // THRE - Transmitter Holding Register Empty
            break;
    }
    asm volatile("out dx, al" : : "a"(c), "d"(_com_port_data));
}

inline void com_write(const char *data, size_t n)
{
    for(int i{}; i < n; ++i)
        com_putc(data[i]);
}

inline char com_getc() 
{
    char result{};

    // Wait for data
    for(uint8_t status{};;)
    {
        // 0x3FD - LSR port
        asm volatile("in al, dx" : "=a"(status) : "d"(_com_port_lsr));
        if(status & 0x1) // DR - Data Ready
            break;
    }

    // Read arrived data
    asm volatile("in al, dx" : "=a"(result) : "d"(_com_port_data));

    return result;
}

#endif // YESELF_COMIO_HPP
