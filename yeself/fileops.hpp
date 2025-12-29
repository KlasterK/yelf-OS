#ifndef YESELF_FILEOPS_HPP
#define YESELF_FILEOPS_HPP

#include "ifile.hpp"

int putc(IFile &file, char c);
int puts(IFile &file, const char *string);
int printf(IFile &file, const char *fmt, ...);
int getc(IFile &file);

#endif // YESELF_FILEOPS_HPP
