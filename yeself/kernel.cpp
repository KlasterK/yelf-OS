#include "common.hpp"
#include "comio.hpp"
#include "vgaio.hpp"

extern "C" void __cdecl c_main()
{
    asm volatile ("cli");

    VGA::Terminal tty;
    tty.puts("Hello World!\n");
    tty.puts("Please input text into COM port and it will be echoed here.\n");

    char input_buf[256];
    char *it;
    for(;;)
    {
        com_readuntil('\r', input_buf, sizeof(input_buf) - 1);

        for(it = input_buf; *it != 0; ++it);
        *it = '\n';
        *++it = 0;

        tty.puts(input_buf);
    }

    for(;;)
        asm volatile ("hlt");
}
