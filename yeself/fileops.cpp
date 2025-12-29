#include "fileops.hpp"
#include "stdarg.h"
#include "common.hpp"

int putc(IFile &file, char c)
{
    return file.write(&c, 1) == 1 ? c : -1;
}

int puts(IFile &file, const char *string)
{
    return file.write(string, strlen(string));
}

int printf(IFile &file, const char *fmt, ...)
{
    auto orig_fmt = fmt;
    va_list args;
    va_start(args, fmt);

    while(*fmt != 0)
    {
        if(*fmt == '\n')
        {
            putc(file, '\r');
        }

        if(*fmt != '%')
        {
            putc(file, *fmt++);
            continue;
        }

        if(*++fmt == '%')
        {
            putc(file, '%');
            ++fmt;
            continue;
        }

        if(*fmt == 'c')
        {
            putc(file, va_arg(args, int));
            ++fmt;
            continue;
        }

        if(*fmt == 's')
        {
            const char *str = va_arg(args, const char *);
            puts(file, str);
            ++fmt;
            continue;
        }

        if(*fmt == 'x' || *fmt == 'X')
        {
            unsigned value = va_arg(args, unsigned int);
            for(int i = 32 - 4; i >= 0; i -= 4)
            {
                int nibble = (value >> i) & 0xF;
                putc(
                    file,
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
    return fmt - orig_fmt;
}

int getc(IFile &file)
{
    char buf{};
    return file.read(&buf, 1) == 1 ? buf : -1;
}
