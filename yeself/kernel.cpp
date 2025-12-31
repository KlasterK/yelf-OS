#include "common.hpp"
#include "comio.hpp"
#include "vgaio.hpp"
#include "kbdio.hpp"
#include "fileops.hpp"

class TTY : public IFile
{
public:
    inline int read(void *buf, size_t n) override { return m_kbd.read(buf, n); };
    inline int write(const void *buf, size_t n) override { return m_vga.write(buf, n); };
    inline int seek(int offset, SeekFrom whence) override { return m_vga.seek(offset, whence); };
    inline int close() override { return 0; };

private:
    VGA::TerminalFile m_vga;
    AT::KeyboardFile  m_kbd;
};

extern "C" void __cdecl c_main()
{
    asm volatile ("cli");

    TTY tty;
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
        tty.write("@ ", 2);
        puts(tty, input_buf);
        putc(tty, '\n');
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
