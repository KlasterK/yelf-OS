#include "common.hpp"
#include "comio.hpp"
#include "vgaio.hpp"
#include "kbdio.hpp"
#include "fileops.hpp"
#include "filewrappers.hpp"
#include "pcidevscanner.hpp"

void print_pci_devs(IFile &out)
{
    PCI::DeviceInfoIterator it;
    PCI::Identification ident;
    PCI::ClassInfo cls;

    while(PCI::next_device_info(it, ident, cls))
    {
        printf(
            out, "PCI %*x:%*x.%*x %*x:%*x Class %*x Subclass %*x ProgIf %*x Rev %*x\n",
            2, it.addr.bus, 
            2, it.addr.device, 
            1, it.addr.function,
            4, ident.vendor_id,
            4, ident.device_id,
            2, cls.base_class, 
            2, cls.subclass, 
            2, cls.programming_interface,
            2, cls.revision_id
        );
    }
}

extern "C" void __cdecl c_main()
{
    asm volatile ("cli");

    VGA::TerminalFile fvga;
    COMPortFile fcom;
    AT::KeyboardFile fkbd;
    ComposedTTY tty(fvga, fkbd);
    puts(tty, "Hello World!\n");

    puts(tty, "Scanning PCI devices...\n");
    print_pci_devs(tty);
    puts(tty, "Scanning has been completed\n");

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
