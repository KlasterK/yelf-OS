#ifndef YESELF_COMIO_HPP
#define YESELF_COMIO_HPP

#include <stdint.h>
#include <stddef.h>

void com_putc(char c);
void com_puts(const char *string);
void com_printf(const char *fmt, ...);
void com_write(const char *data, size_t n);

char com_getc();
bool com_readuntil(char target, char *buffer, size_t n);

#endif // YESELF_COMIO_HPP
