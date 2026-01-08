#include "common.hpp"
#include "comio.hpp"
#include "vgaio.hpp"
#include "kbdio.hpp"
#include "fileops.hpp"
#include "filewrappers.hpp"
#include "pcidevscanner.hpp"
#include "idedmadriver.hpp"
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

static void hexed_ide(IFile &drive, IFile &tty)
{
    char data_buffer[512];
    drive.read(data_buffer, 512);

    for(int i{}; i < 512; i += 8)
    {
        
        if(i % (24 * 8) == 0)
        {
            puts(tty, "Press any key to continue (q = quit) . . . ");
            int gotc = getc(tty);
            putc(tty, '\n');
            
            if(gotc == 'q')
                break;
        }

        Log::printf(
            Log::Info,
            "%x:"
            " %*x %*x %*x %*x %*x %*x %*x %*x"
            " \26%c\26%c\26%c\26%c\26%c\26%c\26%c\26%c",
            i,
            2, data_buffer[i],
            2, data_buffer[i+1],
            2, data_buffer[i+2],
            2, data_buffer[i+3],
            2, data_buffer[i+4],
            2, data_buffer[i+5],
            2, data_buffer[i+6],
            2, data_buffer[i+7],
            data_buffer[i],
            data_buffer[i+1],
            data_buffer[i+2],
            data_buffer[i+3],
            data_buffer[i+4],
            data_buffer[i+5],
            data_buffer[i+6],
            data_buffer[i+7]
        );
    }
}

static void readline_write_ide(IFile &drive, IFile &tty)
{
    char data_buffer[512];
    fill_memory_tml(data_buffer, '\0', sizeof(data_buffer));

    puts(tty, "Data to write: ");
    readline(tty, data_buffer, sizeof(data_buffer));
    drive.write(data_buffer, sizeof(data_buffer));
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

    Log::printf(Log::Info, "Initialising IDE driver (primary drive)...");
    IDE::DriveFile drive(0);
    Log::printf(Log::Info, "IDE driver constructed");

    Log::printf(Log::Info, "Reading from LBA 0... (use keyboard to continue)");
    hexed_ide(drive, tty);

    Log::printf(Log::Info, "Testing LBA 2");
    
    Log::printf(Log::Info, "1. Old data");
    drive.seek(512 * 2, IFile::SeekFrom::Begin);
    hexed_ide(drive, tty);

    Log::printf(Log::Info, "2. Write data");
    drive.seek(-512, IFile::SeekFrom::CurrentPosition);
    readline_write_ide(drive, tty);

    Log::printf(Log::Info, "3. New data");
    drive.seek(-512, IFile::SeekFrom::CurrentPosition);
    hexed_ide(drive, tty);

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
