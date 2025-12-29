#include "common.hpp"
#include "comio.hpp"
#include "vgaio.hpp"
#include "fileops.hpp"

extern "C" void __cdecl c_main()
{
    asm volatile ("cli");

    VGA::TerminalFile vtty;
    COMPortFile ctty;
    puts(vtty, "Hello World!\n");
    puts(vtty, "Please input text into COM port and it will be echoed here.\n");

    char input_buf[256];
    for(;;)
    {
        size_t i{};
        for(; i < sizeof(input_buf) - 1; ++i)
        {
            char c = getc(ctty);
            putc(ctty, c);
            if(c == '\r')
            {
                input_buf[i++] = '\n';
                break;
            }
            else input_buf[i] = c;
        }
        input_buf[i] = 0;

        puts(vtty, input_buf);
    }

    for(;;)
        asm volatile ("hlt");
}

// Stubs

extern "C" void __cxa_pure_virtual()
{
    asm volatile ("cli");
    for(;;)
        asm volatile ("hlt");
}

void operator delete(void*, unsigned int) 
{
    asm volatile ("cli");
    for(;;)
        asm volatile ("hlt");
}
