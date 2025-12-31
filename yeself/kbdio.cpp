#include "kbdio.hpp"
#include "comio.hpp"
#include "fileops.hpp"

using namespace AT;

constexpr int _kbd_port_data = 0x60;
constexpr int _kbd_port_ctl = 0x64;

constexpr int _status_output_full = 0x1;

int KeyboardFile::read(void *buf, size_t n)
{
    auto byte_buf = static_cast<uint8_t *>(buf);
    size_t i{};
    uint8_t scancode{}, kbdstatus{};
    while(i < n)
    {
        for(;;)
        {
            asm volatile("in al, dx" : "=a"(kbdstatus) : "d"(_kbd_port_ctl));
            if(kbdstatus & _status_output_full)
                break;
        }
        asm volatile("in al, dx" : "=a"(scancode) : "d"(_kbd_port_data));

        if(m_pause_idx > 0 && m_pause_idx < (int)sizeof(PauseSequence))
        {
            if(PauseSequence[m_pause_idx++] != scancode)
                m_pause_idx = -1;
            else
                continue;
        }

        if(scancode == PauseSequence[0])
        {
            m_pause_idx = 1;
        }
        else if(m_extension)
        {
            m_extension = 0; // ignore
        }
        else if(m_release)
        {
            m_release = 0;
            uint8_t ascii = (m_shift ? ShiftKeymap : NormalKeymap)[scancode];
            if(ascii == Ctrl)
                m_ctrl = 0;
            else if(ascii == Shift)
                m_shift = 0;
        }
        else if(scancode == 0xE0)
        {
            m_extension = 1;
        }
        else if(scancode == 0xF0)
        {
            m_release = 1;
        }
        else if(scancode < sizeof(NormalKeymap))
        {
            uint8_t ascii = (m_shift ? ShiftKeymap : NormalKeymap)[scancode];
            if(ascii == 0)
                continue;

            if(ascii >= 0x80)
            {
                if(ascii == Ctrl)
                    m_ctrl = 1;
                else if(ascii == Shift)
                    m_shift = 1;
                else if(ascii == CapsLock)
                    m_capslock = !m_capslock;
                continue;
            }
            
            if(m_ctrl)
            {
                if(ascii >= 'a' && ascii <= 'z')
                    ascii = ascii - 'a' + 'A';
                
                if(ascii < 'A' || ascii > '_')
                    continue;
                
                ascii = ascii - '@';
            }
            
            if(m_capslock)
            {
                if(ascii >= 'A' && ascii <= 'Z')
                    ascii = ascii - 'A' + 'a';
                else if(ascii >= 'a' && ascii <= 'z')
                    ascii = ascii - 'a' + 'A';
            }

            byte_buf[i++] = ascii;
        }
        else if(scancode > 0x80)
        {
            m_release = 0;
            uint8_t ascii = (m_shift ? ShiftKeymap : NormalKeymap)[scancode & 0x7F];
            if(ascii == Ctrl)
                m_ctrl = 0;
            else if(ascii == Shift)
                m_shift = 0;
        }
    }
    return i;
}
