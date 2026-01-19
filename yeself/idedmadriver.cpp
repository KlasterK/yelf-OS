#include "idedmadriver.hpp"
#include "pcidevscanner.hpp"
#include "memoryops.hpp"
#include "logging.hpp"

struct PRDE
{
    uint32_t buffer_phys_addr;
    uint16_t byte_count;
    uint16_t reserved : 15;
    uint16_t is_last_table_entry : 1;
} __packed;

namespace RegOff
{
    constexpr uint16_t Command = 0x4;
    constexpr uint16_t BAR0_PriCommandBlock = 0x10;
    constexpr uint16_t BAR1_PriControl = 0x14;
    constexpr uint16_t BAR2_SecCommandBlock = 0x18;
    constexpr uint16_t BAR3_SecControl = 0x1C;
    constexpr uint16_t BAR4_BusMaster = 0x20;
}

namespace BusMastering
{
    constexpr uint16_t BusOffset = 0x8;

    constexpr uint16_t Command = 0x0;
    constexpr uint16_t Status = 0x2;
    constexpr uint16_t PRDTPhysAddrByte0 = 0x4;
    constexpr uint16_t PRDTPhysAddrByte1 = 0x5;
    constexpr uint16_t PRDTPhysAddrByte2 = 0x6;
    constexpr uint16_t PRDTPhysAddrByte3 = 0x7;

    namespace CommandFlags
    {
        constexpr uint8_t StartStop = 1 << 0;
        constexpr uint8_t ReadWrite = 1 << 3; // 0 = write, 1 = read
    }
    namespace StatusFlags
    {
        constexpr uint8_t Active = 1 << 0;
        constexpr uint8_t IRQ    = 1 << 2;
    }
}

namespace ATA
{
    namespace Port
    {
        constexpr uint16_t SectorCount = 0x02;
        constexpr uint16_t LBALow = 0x03;
        constexpr uint16_t LBAMid = 0x04;
        constexpr uint16_t LBAHigh = 0x05;
        constexpr uint16_t Head = 0x06;
        constexpr uint16_t StatusCommand = 0x07;
    }
    namespace Command
    {
        constexpr uint16_t ReadDMA_28bitLBA  = 0xC8;
        constexpr uint16_t WriteDMA_28bitLBA = 0xCA;
    }
    union HeadReg
    {
        uint8_t raw = 0b11100000;
        struct
        {
            uint8_t lba_upper_bits : 4;
            uint8_t is_slave       : 1;
            uint8_t always1        : 1;
            uint8_t do_use_lba     : 1;
            uint8_t always1_       : 1;
        } __packed;
    } __packed;
}

constexpr uint8_t ClassMassStorage = 0x1;
constexpr uint8_t SubclassIDEController = 0x1;
constexpr uint8_t ProgIfBusMasteringBit = 0x80;

// Remember of 64 KiB boundary
alignas(512) static uint8_t _dma_buffer[512];

// TODO: make an allocator for PRDTables
static PRDE _prdt[] = {
    // NOTE: when we will introduce virtual memory, simple address taking won't work
    {reinterpret_cast<uint32_t>(_dma_buffer), sizeof(_dma_buffer), 0, 1}
};

static bool g_object_counter{};

static bool _find_ide(PCI::Address &out_addr)
{
    PCI::DeviceInfoIterator it;
    PCI::Identification ident;
    PCI::ClassInfo cls;

    while(PCI::next_device_info(it, ident, cls))
    {
        if (cls.base_class == ClassMassStorage
            && cls.subclass == SubclassIDEController
            && cls.programming_interface & ProgIfBusMasteringBit)
        {
            out_addr = it.addr;
            return true;
        }
    }
    return false;
}

static uint16_t _get_port_base(PCI::Address addr, uint32_t regoff)
{
    addr.register_offset = regoff;
    Log::printf(Log::Trace, "Regoff %*x Device %*x", 2, addr.register_offset, 2, addr.device);
    uint32_t port = PCI::device_get(addr);
    Log::printf(Log::Trace, "Port from pci: %x", port);
    return port & 0xFFFC;
}

static bool _enable_bm(PCI::Address addr)
{
    addr.register_offset = RegOff::Command;

    uint16_t command_data = PCI::device_get(addr) & 0xFFFF;
    command_data |= 0x05; // Bit 2 = Bus Master Enable, Bit 0 = IO Space Enable
    PCI::device_put(addr, command_data);

    // Check if command recved
    command_data = PCI::device_get(addr) & 0xFFFF;
    return command_data & 0x4 && command_data & 0x1;
}

// We count that bm_port_base is already summed with bus offset
static void _setup_bm(uint16_t bm_port_base, uint32_t prdt_phys_addr, bool is_secondary_bus)
{
    if(is_secondary_bus)
        bm_port_base += BusMastering::BusOffset;

    asm volatile("out dx, al" : : "a"(prdt_phys_addr & 0xFF), 
                 "d"(bm_port_base + BusMastering::PRDTPhysAddrByte0));

    asm volatile("out dx, al" : : "a"(prdt_phys_addr >> 8 & 0xFF), 
                 "d"(bm_port_base + BusMastering::PRDTPhysAddrByte1));

    asm volatile("out dx, al" : : "a"(prdt_phys_addr >> 16 & 0xFF), 
                 "d"(bm_port_base + BusMastering::PRDTPhysAddrByte2));

    asm volatile("out dx, al" : : "a"(prdt_phys_addr >> 24 & 0xFF), 
                 "d"(bm_port_base + BusMastering::PRDTPhysAddrByte3));

    // Remove all flags
    asm volatile("out dx, al" : : "a"(0), "d"(bm_port_base + BusMastering::Status));
}

static void _ata_begin_transaction(
    uint16_t command_block_port_base, uint16_t bm_port_base, 
    bool is_slave, uint32_t lba, uint8_t sector_count,
    bool is_read
)
{
    asm volatile("out dx, al" : : "a"(lba & 0xFF), 
                 "d"(command_block_port_base + ATA::Port::LBALow));
                 
    asm volatile("out dx, al" : : "a"(lba >> 8 & 0xFF), 
                 "d"(command_block_port_base + ATA::Port::LBAMid));
                 
    asm volatile("out dx, al" : : "a"(lba >> 16 & 0xFF), 
                 "d"(command_block_port_base + ATA::Port::LBAHigh));
                 
    ATA::HeadReg head{
        .always1=1, .always1_=1, .do_use_lba=1, .is_slave=is_slave,
        .lba_upper_bits=uint8_t(command_block_port_base >> 24 & 0x0F)
    };

    asm volatile("out dx, al" : : "a"(head.raw), 
                 "d"(command_block_port_base + ATA::Port::Head));

    asm volatile("out dx, al" : : "a"(sector_count), 
                 "d"(command_block_port_base + ATA::Port::SectorCount));

    asm volatile("out dx, al" : : "a"(is_read ? ATA::Command::ReadDMA_28bitLBA 
                                              : ATA::Command::WriteDMA_28bitLBA), 
                 "d"(command_block_port_base + ATA::Port::StatusCommand));

    asm volatile("wbinvd" : : : "memory");

    uint8_t cmd{};
    asm volatile("in al, dx" : "=a"(cmd) : "d"(bm_port_base + BusMastering::Command));
    cmd |= BusMastering::CommandFlags::StartStop;
    if(is_read)
        cmd |= BusMastering::CommandFlags::ReadWrite;
    else
        cmd &= ~BusMastering::CommandFlags::ReadWrite;
    asm volatile("out dx, al" : : "a"(cmd), "d"(bm_port_base + BusMastering::Command));
}

static void _ata_poll_dma(uint16_t bm_port_base)
{
    uint8_t status{};
    for(;;)
    {
        asm volatile("in al, dx" : "=a"(status) : "d"(bm_port_base + BusMastering::Status));
        if(uint8_t flag = status & BusMastering::StatusFlags::Active; flag == 0)
            break;
    }
}

static void _ata_end_transaction(uint16_t bm_port_base)
{
    uint8_t status{};
    asm volatile("in al, dx" : "=a"(status) : "d"(bm_port_base + BusMastering::Status));
    status &= ~BusMastering::StatusFlags::Active;
    status |=  BusMastering::StatusFlags::IRQ;
    asm volatile("out dx, al" : : "a"(status), "d"(bm_port_base + BusMastering::Status));

    uint8_t command{};
    asm volatile("in al, dx" : "=a"(command) : "d"(bm_port_base + BusMastering::Command));
    command &= ~BusMastering::CommandFlags::StartStop;
    asm volatile("out dx, al" : : "a"(command), "d"(bm_port_base + BusMastering::Command));

    asm volatile("wbinvd" : : : "memory");
}

IDE::DriveFile::DriveFile(bool is_secondary, bool is_slave)
    : m_is_slave(is_slave)
    , m_lba(0)
{
    if(g_object_counter)
    {
        Log::printf(Log::Error, "IDE::DriveFile is a singleton, cannot create 2nd instance");
        return;
    }
    g_object_counter = true;

    auto addr = PCI::Address::from_components(0, 0, 0, 0);
    if(!_find_ide(addr))
    {
        Log::printf(Log::Warning, "_find_ide returned 0");
        return;
    }

    if(!is_secondary)
    {
        m_command_block_port_base = _get_port_base(addr, RegOff::BAR0_PriCommandBlock);
        if(m_command_block_port_base == 0)
            m_command_block_port_base = 0x1F0;

        m_control_port_base = _get_port_base(addr, RegOff::BAR1_PriControl);
        if(m_control_port_base == 0)
            m_control_port_base = 0x3F6;
    }
    else
    {
        m_command_block_port_base = _get_port_base(addr, RegOff::BAR2_SecCommandBlock);
        if(m_command_block_port_base == 0)
            m_command_block_port_base = 0x170;
        
        m_control_port_base = _get_port_base(addr, RegOff::BAR3_SecControl);
        if(m_control_port_base == 0)
            m_control_port_base = 0x376;
    }

    m_bm_port_base = _get_port_base(addr, RegOff::BAR4_BusMaster);
    Log::printf(Log::Debug, "IDE ports: Block %x Control %x BM %x", m_command_block_port_base, m_control_port_base, m_bm_port_base);
    _enable_bm(addr);
    _setup_bm(m_bm_port_base, reinterpret_cast<uint32_t>(_prdt), is_secondary);
}

IDE::DriveFile::~DriveFile()
{
    g_object_counter = false;
}

int IDE::DriveFile::read(void *buf, size_t n)
{
    if(n % 512 != 0)
        return -1;
    
    if(n == 0)
        return 0;

    auto it = static_cast<uint8_t *>(buf);
    auto end = it + n;
    for(; it < end; it += 512)
    {
        _ata_begin_transaction(m_command_block_port_base, m_bm_port_base, m_is_slave, m_lba++, 1, true);
        _ata_poll_dma(m_bm_port_base);
        _ata_end_transaction(m_bm_port_base);

        copy_memory_tml(it, _dma_buffer, 512);
    }

    return n;
}

int IDE::DriveFile::write(const void *buf, size_t n)
{
    if(n % 512 != 0)
        return -1;
    
    if(n == 0)
        return 0;

    auto it = static_cast<const uint8_t *>(buf);
    auto end = it + n;
    for(; it < end; it += 512)
    {
        copy_memory_tml(_dma_buffer, it, 512);

        _ata_begin_transaction(m_command_block_port_base, m_bm_port_base, m_is_slave, m_lba++, 1, false);
        _ata_poll_dma(m_bm_port_base);
        _ata_end_transaction(m_bm_port_base);
    }

    return n;
}

int IDE::DriveFile::seek(int offset, SeekFrom whence)
{
    if(offset % 512 != 0)
        return -1;

    if (whence == SeekFrom::Begin)
        m_lba = max(0, offset / 512);
    else if (whence == SeekFrom::CurrentPosition)
        m_lba = (uint32_t)clamp((int32_t)m_lba + offset / 512, 0, 1 << 28);
    else if (whence == SeekFrom::End)
        return -1; // TODO: implement disk size retrieving

    return m_lba * 512;
}
