#ifndef YESELF_FILEOPS_HPP
#define YESELF_FILEOPS_HPP

#include "ifile.hpp"
#include <stdarg.h>

int putc(IFile &file, char c);
int puts(IFile &file, const char *string);
int printf_va(IFile &file, const char *fmt, va_list args);
int printf(IFile &file, const char *fmt, ...);
int getc(IFile &file);
char *gets(IFile &file, char *buf, size_t n);
char *readline(IFile &file, char *buf, size_t n, size_t initial_text_length = 0);

#endif // YESELF_FILEOPS_HPP
