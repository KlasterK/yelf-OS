#include "common.hpp"
#include "comio.hpp"
#include "vgaio.hpp"
#include "kbdio.hpp"
#include "fileops.hpp"
#include "filewrappers.hpp"
#include "pcidevscanner.hpp"
#include "logging.hpp"
#include "memoryops.hpp"

static void log_pci_devs()
{
    PCI::DeviceInfoIterator it;
    PCI::Identification ident;
    PCI::ClassInfo cls;

    while(PCI::next_device_info(it, ident, cls))
    {
        Log::printf(
            Log::Info, "PCI %*x:%*x.%*x %*x:%*x Class %*x Subclass %*x ProgIf %*x Rev %*x",
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

    Log::Level log_table[] = {
        {Log::Trace,    "TRACE",    &fcom, nullptr},
        {Log::Debug,    "DEBUG",    &fcom, nullptr},
        {Log::Info,     "INFO",     &fcom, &fvga},
        {Log::Warning,  "WARNING",  &fcom, &fvga},
        {Log::Error,    "ERROR",    &fcom, &fvga},
        {Log::Critical, "CRITICAL", &fcom, &fvga},
    };
    Log::init(log_table, sizeof(log_table) / sizeof(*log_table));

    Log::printf(Log::Info, "Hello World!");

    Log::printf(Log::Info, "Scanning PCI devices...");
    log_pci_devs();
    Log::printf(Log::Info, "Scanning has been completed");

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
    Log::panic("unimplemented __cxa_pure_virtual called, terminating.");
}

void operator delete(void*, unsigned int) 
{
    Log::panic("unimplemented operator delete called, terminating.");
}
