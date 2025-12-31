#ifndef YESELF_KBDIO_HPP
#define YESELF_KBDIO_HPP

#include "common.hpp"
#include "ifile.hpp"
#include <stdint.h>
#include <stddef.h>

namespace AT
{

constexpr uint8_t Ctrl = 0x80, Shift = 0x81, CapsLock = 0x82;

constexpr uint8_t NormalKeymap[] = {
    "\0" // Null
    "\e" // Esc
    "1234567890-=\b"
    "\tqwertyuiop[]\n"
    "\x80" // LCtrl
    "asdfghjkl;'`"
    "\x81" // LShift
    "\\zxcvbnm,./"
    "\x81" // RShift
    "*"    // Keypad Mul
    "\0"   // LAlt
    " "    // Space
    "\x82" // Caps Lock
    "\0\0\0\0\0\0\0\0\0\0" // F1--F10
    "\0" // Num Lock
    "\0" // Scroll Lock
    "789-456+1230." // Keypad
    "\0\0\0" // Alt+SysRq, unused codes
    "\0\0"   // F11--F12
};

constexpr uint8_t ShiftKeymap[] = {
    "\0\e!@#$%^&*()_+\b\tQWERTYUIOP{}\n\x80"
    "ASDFGHJKL:\"~\x81|ZXCVBNM<>?\x81*\0"
    " \x82\0\0\0\0\0\0\0\0\0\0\0\0"
    "789-567+1230.\0\0\0\0\0"
};

class KeyboardFile : public IFile
{
public:
    int read(void *buf, size_t n) override;
    inline int write(const void *, size_t) override { return -1; };
    inline int seek(int, SeekFrom) override { return -1; };
    inline int close() override { return 0; };

private:
    bool m_ctrl{}, m_shift{}, m_capslock{}, m_release{}, m_extension{};
    int m_pause_idx = -1;
    
    static constexpr uint8_t PauseSequence[] = "\xE1\x14\x77\xE1\xF0\x14\xF0\x77";
};

} // namespace AT

#endif // YESELF_KBDIO_HPP
