#ifndef YESELF_IDEDMADRIVER_HPP
#define YESELF_IDEDMADRIVER_HPP

#include "common.hpp"
#include "ifile.hpp"

namespace IDE
{

constexpr size_t BlockSize = 512;

class DriveFile : public IFile
{
public:
    DriveFile(uint8_t drive_num);
    ~DriveFile() override;
    int read(void *buf, size_t n) override;
    int write(const void *buf, size_t n) override;
    int seek(int offset, SeekFrom whence) override;
    inline int close() override { return 0; }

private:
    uint16_t m_command_block_port_base{};
    uint16_t m_control_port_base{};
    uint16_t m_bm_port_base{};
    uint8_t  m_drive{};
    uint32_t m_lba{};
};

} // namespace IDE

#endif // YESELF_IDEDMADRIVER_HPP
