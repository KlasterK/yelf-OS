#ifndef __cdecl
    #define __cdecl __attribute__((cdecl))
#endif

extern "C" void __cdecl a_com_putc(char);

extern "C" void __cdecl c_main()
{
    a_com_putc('H');
    a_com_putc('\n');
    for(;;) {}
}
