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
        ctty.write("> ", 2);
        if(readline(ctty, input_buf, sizeof(input_buf)) == nullptr)
        {
            puts(vtty, "\nEOF received\n");
            break;
        }
        puts(vtty, input_buf);
        putc(vtty, '\n');
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
