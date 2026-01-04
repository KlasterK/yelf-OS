#include "fileops.hpp"
#include "common.hpp"

// TODO: implement a better algorithm for printing decimals
static void _print_decimal_recursive(IFile &file, int num)
{
    if(num < 0)
    {
        putc(file, '-');
        num = -num;
    }

    if(num >= 10)
        _print_decimal_recursive(file, num / 10);
    
    putc(file, '0' + num % 10);
}

int putc(IFile &file, char c)
{
    return file.write(&c, 1) == 1 ? c : -1;
}

int puts(IFile &file, const char *string)
{
    return file.write(string, strlen(string));
}

int printf_va(IFile &file, const char *fmt, va_list args)
{
    auto orig_fmt = fmt;
    int star_argument = -1;

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

        if(*++fmt == '*')
        {
            // Star means get %-specifier argument from the stack
            star_argument = va_arg(args, int);
            ++fmt;
        }

        if(*fmt == '%')
        {
            putc(file, '%');
        }
        else if(*fmt == 'c')
        {
            putc(file, va_arg(args, int));
        }
        else if(*fmt == 's')
        {
            const char *str = va_arg(args, const char *);
            puts(file, str);
        }
        else if(*fmt == 'x' || *fmt == 'X')
        {
            int nibbles_begin = 32 - 4;
            if(star_argument > 0 && star_argument < 8)
            {
                nibbles_begin -= 4 * (8 - star_argument);
            }

            unsigned value = va_arg(args, unsigned int);
            for(int i = nibbles_begin; i >= 0; i -= 4)
            {
                int nibble = (value >> i) & 0xF;
                putc(
                    file,
                    nibble >= 0xA
                    ? (*fmt == 'X' ? 'A' : 'a') - 0xA + nibble 
                    : '0' + nibble
                );
            }
        }
        else if(*fmt == 'd')
        {
            unsigned value = va_arg(args, int);
            _print_decimal_recursive(file, value);
        }

        ++fmt;
        star_argument = -1;
    }
    return fmt - orig_fmt;
}

int printf(IFile &file, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = printf_va(file, fmt, args);
    va_end(args);
    return ret;
}

int getc(IFile &file)
{
    char buf{};
    return file.read(&buf, 1) == 1 ? buf : -1;
}

char *gets(IFile &file, char *buf, size_t n)
{
    char input_buf{};
    size_t i{};
    for(; i < n-1; ++i)
    {
        if(file.read(&input_buf, 1) != 1)
        {
            buf[i] = 0;
            return nullptr;
        }
        if(input_buf == '\n' || input_buf == '\r')
        {
            buf[i] = 0;
            return buf;
        }
        buf[i] = input_buf;
    }
    buf[i] = 0;
    return nullptr;
}

char *readline(IFile &file, char *buf, size_t n, size_t initial_text_length)
{
    char input_buf{};
    size_t i = initial_text_length;

    while(i < n-1)
    {
        if(file.read(&input_buf, 1) != 1)
        {
            buf[i] = 0;
            return nullptr;
        }
        if(input_buf == '\n' || input_buf == '\r')
        {
            putc(file, '\n');
            buf[i] = 0;
            return buf;
        }
        if(input_buf == 'D' - '@')
        {
            file.close();
            buf[i] = 0;
            return nullptr;
        }

        if(input_buf == '\b' || input_buf == '\x7F') // 7F = DEL
        {
            if(i > 0)
            {
                --i;
                file.write("\b \b", 3);
            }
        }
        else if(input_buf == 'U' - '@')
        {
            for(; i > 0; --i)
                file.write("\b \b", 3);
        }
        else if(input_buf == '\e')
        {
            if(file.read(&input_buf, 1) != 1)
            {
                buf[i] = 0;
                return nullptr;
            }
            if(input_buf == '[')
            {
                for(;;)
                {
                    if(file.read(&input_buf, 1) != 1)
                    {
                        buf[i] = 0;
                        return nullptr;
                    }
                    if(input_buf >= 0x40 && input_buf <= 0x7E)
                        break;
                }
            }
        }
        else 
        {
            buf[i++] = input_buf;
            putc(file, input_buf);
        }
    }

    buf[i] = 0;
    return nullptr;
}
