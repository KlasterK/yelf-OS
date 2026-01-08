#include "logging.hpp"
#include "fileops.hpp"
#include "vgaio.hpp"

Log::Level *g_table{};
size_t g_size{};

void Log::init(Level *table, size_t size)
{
    g_table = table;
    g_size = size;
}

bool Log::printf(int value, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int result = Log::printf_va(value, fmt, args);
    va_end(args);
    return result;
}

bool Log::printf_va(int value, const char *fmt, va_list args)
{
    if(g_table == nullptr)
        return false;

    for(size_t i{}; i < g_size; ++i)
    {
        Log::Level level = g_table[i];
        if(level.value != value)
            continue;
        
        bool ok = true, do_any_file_exist = false;
        va_list args2;
        va_copy(args2, args);

        if(level.primary_file != nullptr)
        {
            do_any_file_exist = true;
            bool a = 0 <= printf(*level.primary_file, "[%s] ", level.name);
            bool b = 0 <= printf_va(*level.primary_file, fmt, args);
            bool c = 0 <= putc(*level.primary_file, '\n');
            ok = ok && a && b && c;
        }

        if(level.secondary_file != nullptr)
        {
            do_any_file_exist = true;
            bool a = 0 <= printf(*level.secondary_file, "[%s] ", level.name);
            bool b = 0 <= printf_va(*level.secondary_file, fmt, args2);
            bool c = 0 <= putc(*level.secondary_file, '\n');
            ok = ok && a && b && c;
        }

        va_end(args2);
        return ok && do_any_file_exist;
    }

    return false;
}

void Log::panic(const char *fmt, ...)
{
    va_list args, args2;
    va_start(args, fmt);

    // 1. Try to log panic in default levels
    va_copy(args2, args);
    if(Log::printf_va(Log::Critical, fmt, args2))
    {
        Log::printf(Log::Critical, "KERNEL PANIC: write this error and reset your PC.");
        asm volatile ("cli");
        for(;;)
            asm volatile ("hlt");
    }
    va_end(args2);

    va_copy(args2, args);
    if(Log::printf_va(Log::Error, fmt, args2))
    {
        Log::printf(Log::Error, "KERNEL PANIC: write this error and reset your PC.");
        asm volatile ("cli");
        for(;;)
            asm volatile ("hlt");
    }
    va_end(args2);

    // 2. Try to log panic in any level
    for(size_t i{}; i < (g_table == nullptr ? 0 : g_size); ++i)
    {
        Log::Level level = g_table[i];
        if(level.primary_file != nullptr || level.secondary_file != nullptr)
            continue;
        
        va_copy(args2, args);
        if(Log::printf_va(level.value, fmt, args2))
        {
            Log::printf(level.value, "KERNEL PANIC: write this error and reset your PC.");
            asm volatile ("cli");
            for(;;)
                asm volatile ("hlt");
        }
        va_end(args2);
    }

    // 3. Write directly to VGA text mode video memory
    auto vgamem = (volatile VGA::Char *)0xB8000;
    for(auto it = fmt; *it != 0; ++it)
        *(VGA::Char *)vgamem++ = VGA::Char{.style=VGA::CharStyle{}, .text=*it};
    
    for(size_t i{}; i < 80; ++i)
        *(VGA::Char *)vgamem++ = VGA::Char{.style=VGA::CharStyle{}, .text=' '};

    for(auto it = "KERNEL PANIC: write this error and reset your PC."; *it != 0; ++it)
        *(VGA::Char *)vgamem++ = VGA::Char{.style=VGA::CharStyle{}, .text=*it};

    asm volatile ("cli");
    for(;;)
        asm volatile ("hlt");
}
