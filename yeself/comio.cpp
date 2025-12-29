#include "comio.hpp"
#include "common.hpp"
#include <stdarg.h>

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
            com_putc(va_arg(args, char));
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
