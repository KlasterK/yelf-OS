#ifndef YESELF_PCIDEVSCANNER_HPP
#define YESELF_PCIDEVSCANNER_HPP

#include "common.hpp"
#include "ifile.hpp"
#include <stdint.h>
#include <stddef.h>

namespace PCI
{

union Address
{
    uint32_t raw;
    struct
    {
        uint32_t register_offset    : 8;
        uint32_t function           : 3;
        uint32_t device             : 5;
        uint32_t bus                : 8;
        uint32_t reserved           : 7;
        uint32_t do_enable          : 1;
    } __packed;

    static inline Address from_components(
        uint32_t bus, uint32_t device, 
        uint32_t function, uint32_t register_offset
    ) { return 
    {
        .bus=bus, .device=device, .function=function, 
        .register_offset=register_offset,
        .reserved=0, .do_enable=1,
    }; }
} __packed;

union Identification 
{
    uint32_t raw;
    struct 
    {
        uint16_t vendor_id;
        uint16_t device_id;
    } __packed;
    
    inline bool exists() const { return vendor_id != 0xFFFF; }
    static constexpr uint32_t RegisterID = 0x0;
};

union ClassInfo 
{
    uint32_t raw;
    struct 
    {
        uint8_t revision_id;
        uint8_t programming_interface;
        uint8_t subclass;
        uint8_t base_class;
    } __packed;
    static constexpr uint32_t RegisterID = 0x8;
};

uint32_t device_get(Address addr);
void device_put(Address addr, uint32_t value);

struct DeviceInfoIterator
{
    Address addr = Address::from_components(0, 0, 0, 0);
    bool do_need_fn_increment{};
};
bool next_device_info(DeviceInfoIterator &it, Identification &out_ident, ClassInfo &out_cls);

} // namespace PCI

#endif // YESELF_PCIDEVSCANNER_HPP
