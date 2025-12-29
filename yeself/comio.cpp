#include "comio.hpp"
#include "common.hpp"
#include <stdarg.h>

constexpr int _com_port_data = 0x3F8;
constexpr int _com_port_lsr  = _com_port_data + 5;

void com_putc(char c)
{
    for(uint8_t status{};;)
    {
        asm volatile("in al, dx" : "=a"(status) : "d"(_com_port_lsr));
        if(status & 0x20) // THRE - Transmitter Holding Register Empty
            break;
    }
    asm volatile("out dx, al" : : "a"(c), "d"(_com_port_data));
}

void com_puts(const char *string)
{
    while(*string != 0)
        com_putc(*string++);
}

void com_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while(*fmt != 0)
    {
        if(*fmt == '\n')
        {
            com_putc('\r');
        }

        if(*fmt != '%')
        {
            com_putc(*fmt++);
            continue;
        }

        if(*++fmt == '%')
        {
            com_putc('%');
            ++fmt;
            continue;
        }

        if(*fmt == 'c')
        {
            com_putc(va_arg(args, int));
            ++fmt;
            continue;
        }

        if(*fmt == 's')
        {
            const char *str = va_arg(args, const char *);
            while(*str != 0)
                com_putc(*str++);
            ++fmt;
            continue;
        }

        if(*fmt == 'x' || *fmt == 'X')
        {
            unsigned value = va_arg(args, unsigned int);
            for(int i = 32 - 4; i >= 0; i -= 4)
            {
                int nibble = (value >> i) & 0xF;
                com_putc(
                    nibble >= 0xA
                    ? (*fmt == 'X' ? 'A' : 'a') - 0xA + nibble 
                    : '0' + nibble
                );
            }
            ++fmt;
            continue;
        }
    }

    va_end(args);
}

void com_write(const char *data, size_t n)
{
    for(size_t i{}; i < n; ++i)
        com_putc(data[i]);
}

char com_getc() 
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

bool com_readuntil(char target, char *buffer, size_t n)
{
    char input{};
    size_t i{};

    for(; i < n; ++i)
    {
        input = com_getc();
        com_putc(input);
        if(input == target)
        {
            buffer[i] = 0;
            return true;
        }
        buffer[i] = input;
    }

    buffer[i] = 0;
    return false;
}
