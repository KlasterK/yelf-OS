#include "pcidevscanner.hpp"
#include "comio.hpp"
#include "fileops.hpp"

constexpr uint16_t _pci_port_dev_addr = 0xCF8;
constexpr uint16_t _pci_port_dev_data = 0xCFC;

uint32_t PCI::device_get(Address addr)
{
    uint32_t value{};
    asm volatile ("out dx, eax" : : "a"(addr.raw), "d"(_pci_port_dev_addr));
    asm volatile ("in eax, dx" : "=a"(value) : "d"(_pci_port_dev_data));
    return value;
}

void PCI::device_put(Address addr, uint32_t value)
{
    asm volatile ("out dx, eax" : : "a"(addr.raw), "d"(_pci_port_dev_addr));
    asm volatile ("out dx, eax" : : "a"(value), "d"(_pci_port_dev_data));
}

bool PCI::next_device_info(DeviceInfoIterator &it, Identification &out_ident, ClassInfo &out_cls)
{
    for(;;) // bus loop
    {
        for(;;) // device loop
        {
            if(it.do_need_fn_increment)
            {
                if(it.addr.function < 7)
                {
                    ++it.addr.function;
                }
                else
                {
                    it.addr.function = 0;
                    break;
                }
            }

            // Check fn 0 first
            if(it.addr.function == 0)
            {
                it.addr.register_offset = Identification::RegisterID;
                out_ident.raw = device_get(it.addr);
                if(!out_ident.exists())
                    continue;
            }

            // If fn 0 exists, then iterate through everything
            for(;;) // function loop
            {
                it.addr.register_offset = Identification::RegisterID;
                out_ident.raw = device_get(it.addr);
                if(!out_ident.exists())
                {
                    if(it.addr.function < 7)
                    {
                        ++it.addr.function;
                        continue;
                    }
                    else
                    {
                        it.addr.function = 0;
                        break;
                    }
                }
                
                it.addr.register_offset = ClassInfo::RegisterID;
                out_cls.raw = device_get(it.addr);

                it.do_need_fn_increment = true;
                return true;
            }

            if(it.addr.device < 31)
            {
                ++it.addr.device;
            }
            else
            {
                ++it.addr.device;
                break;
            }
        }

        if(it.addr.bus < 255)
        {
            ++it.addr.bus;
        }
        else
        {
            it.addr.device = 0;
            ++it.addr.bus;
            break;
        }
    }
    return false;
}
