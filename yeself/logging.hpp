#ifndef YESELF_LOGGING_HPP
#define YESELF_LOGGING_HPP

#include "common.hpp"
#include "ifile.hpp"
#include <stdarg.h>

namespace Log
{
    struct Level
    {
        int value;
        const char *name;
        IFile *primary_file;
        IFile *secondary_file;
    };

    void init(Level *table, size_t size);
    bool printf(int value, const char *fmt, ...);
    bool printf_va(int value, const char *fmt, va_list args);
    [[noreturn]] void panic(const char *fmt, ...);

    constexpr int Trace     = 10;
    constexpr int Debug     = 20;
    constexpr int Info      = 30;
    constexpr int Warning   = 40;
    constexpr int Error     = 40;
    constexpr int Critical  = 50;
} // namespace Log

#endif // YESELF_LOGGING_HPP
