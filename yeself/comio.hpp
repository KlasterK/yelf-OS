#ifndef YESELF_COMIO_HPP
#define YESELF_COMIO_HPP

#include "ifile.hpp"

class COMPortFile : public IFile
{
public:
    int read(void *buf, size_t n) override;
    int write(const void *buf, size_t n) override;
    inline int seek(int, SeekFrom) override { return -1; };
    inline int close() override { return 0; };
};

#endif // YESELF_COMIO_HPP
