#ifndef YESELF_TARFS_HPP
#define YESELF_TARFS_HPP

#include "common.hpp"
#include "ifile.hpp"
#include "idirectory.hpp"
#include "stl.hpp"

namespace TAR
{

enum class MountError
{
    Read, Signature, Version
};

constexpr size_t BlockSize = 512;
constexpr uint8_t Signature[8] = {'y', 'e', 'l', 'f', 't', 'a', 'r', '.'};
constexpr uint32_t Version = 1;

struct SystemHeader
{
    uint8_t signature[8];
    uint32_t version;
} __packed;
static_assert(sizeof(SystemHeader) < BlockSize);

struct FileHeader
{
    char name[64];
    uint32_t size;
    IDirectory::NodeType type;
    uint8_t unused_a[3];
} __packed;
static_assert(sizeof(IDirectory::NodeType) == 1);
static_assert(sizeof(FileHeader) % 8 == 0);
static_assert(sizeof(FileHeader) < BlockSize / 2);

class File : public IFile
{
public:
    int read(void *buf, size_t n) override;
    inline int write(const void *, size_t) override { return -1; };
    int seek(int offset, SeekFrom whence) override;
    inline int close() override { return 0; };

private:
    File(IFile &partition, size_t lba);
    IFile &m_partition;
    uint32_t m_lba{}, m_size{}, m_cursor{};

    friend class Directory;
};

class Directory : public IDirectory
{
public:
    Directory(const Directory &) = delete;
    Directory(Directory &&) = default;

    Directory &operator=(const Directory &) = delete;
    inline Directory &operator=(Directory &&) = default;

    Iterator next(Iterator prev_it) const override;
    Iterator find(const char *name) const override;

    inline Iterator create_file(const char *) override { return -1; };
    inline Iterator create_directory(const char *) override { return -1; };
    inline bool remove(Iterator) override { return false; };

    size_t get_name_length(Iterator it) const override;
    size_t get_name(Iterator it, char *buf, size_t size) const override;
    NodeType get_node_type(Iterator it) const override;

    size_t get_node_object_size(Iterator it) override;
    void *open(Iterator it, /* alignas 16 */ void *buf) override;

private:
    Directory(IFile &partition, size_t lba);
    IFile *m_partition;
    size_t m_lba{}, m_records_count{};

    constexpr bool is_existing_iterator(int iterator) const;
    friend Variant<Directory, MountError> mount(IFile &partition);
};

Variant<Directory, MountError> mount(IFile &partition);

} // namespace TAR

#endif // YESELF_TARFS_HPP
