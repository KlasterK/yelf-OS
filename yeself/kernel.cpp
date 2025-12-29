#include "common.hpp"
#include "comio.hpp"

extern "C" void __cdecl c_main()
{
    com_printf("Hello World!\n");
    com_printf("Let's see what is at 0... 0x%X\n", *(volatile int *)0);
    com_printf("Press any key to hang up . . . ");
    char key = com_getc();
    com_printf("\nYour key was %c. %s...\n", key, "Hanging up");
    
    asm volatile ("cli");
    for(;;)
        asm volatile ("hlt");
}
