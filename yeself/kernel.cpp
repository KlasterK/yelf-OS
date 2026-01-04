#include "common.hpp"
#include "comio.hpp"
#include "vgaio.hpp"
#include "kbdio.hpp"
#include "fileops.hpp"
#include "filewrappers.hpp"

extern "C" void __cdecl c_main()
{
    asm volatile ("cli");

    VGA::TerminalFile fvga;
    COMPortFile fcom;
    AT::KeyboardFile fkbd;
    ComposedTTY tty(fvga, fkbd);
    puts(tty, "Hello World!\n");

    char input_buf[256];
    for(;;)
    {
        tty.write("> ", 2);
        if(readline(tty, input_buf, sizeof(input_buf)) == nullptr)
        {
            puts(tty, "\nEOF received\n");
            break;
        }
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
