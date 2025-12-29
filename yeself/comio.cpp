#include "comio.hpp"
#include <stdint.h>

constexpr int _com_port_data = 0x3F8;
constexpr int _com_port_lsr  = _com_port_data + 5;

int COMPortFile::read(void *buf, size_t n)
{
    auto char_buf = static_cast<char *>(buf);
    char com_input{};
    size_t i{};
    uint8_t status{};

    for(; i < n; ++i)
    {
        for(;;)
        {
            asm volatile("in al, dx" : "=a"(status) : "d"(_com_port_lsr));
            if(status & 0x1) // DR - Data Ready
                break;
        }

        asm volatile("in al, dx" : "=a"(com_input) : "d"(_com_port_data));

        char_buf[i] = com_input;
    }

    return i;
}

int COMPortFile::write(const void *buf, size_t n)
{
    auto char_buf = static_cast<const char *>(buf);
    char com_output{};
    size_t i{};
    uint8_t status{};

    for(; i < n; ++i)
    {
        com_output = char_buf[i];
        for(;;)
        {
            asm volatile("in al, dx" : "=a"(status) : "d"(_com_port_lsr));
            if(status & 0x20) // THRE - Transmitter Holding Register Empty
                break;
        }
        asm volatile("out dx, al" : : "a"(com_output), "d"(_com_port_data));
    }

    return i;
}
