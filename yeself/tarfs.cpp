#include "tarfs.hpp"
#include "memoryops.hpp"
#include "stringops.hpp"
#include "logging.hpp"

using namespace TAR;


static int _read_lba(IFile &partition, uint32_t lba, void *buf, size_t nblocks = 1)
{
    if(partition.seek(lba * BlockSize, IFile::SeekFrom::Begin) < 0)
        return -1;
    return partition.read(buf, nblocks * BlockSize);
}


static FileHeader &_file_hdr_of(void *buf)
{
    return *reinterpret_cast<FileHeader *>(buf);
}


static constexpr size_t file_size_to_blocks_count(size_t size)
{
    constexpr size_t FirstBlockDataSize = BlockSize - sizeof(FileHeader);
    if(size <= FirstBlockDataSize)
        return 1;
    size_t NextBlocksDataSize = size - FirstBlockDataSize;
    return 1 + ceildiv(NextBlocksDataSize, BlockSize);
}


constexpr bool Directory::is_existing_iterator(Directory::Iterator iterator) const
{
    return iterator >= ((int32_t)sizeof(FileHeader) / 4)
        && (uint32_t)iterator < (sizeof(FileHeader) / 4 + m_records_count - 1);
}


static constexpr uint32_t _dirit2lba(Directory::Iterator iterator, uint32_t base_lba)
{
    return base_lba + iterator / (BlockSize / 4);
}


static constexpr uint32_t _dirit2offset(Directory::Iterator iterator)
{
    return iterator % (BlockSize / 4);
}


static constexpr Directory::Iterator _make_dirit(uint32_t lba, uint32_t offset)
{
    return lba * (BlockSize / 4) + offset;
}


File::File(IFile &partition, size_t lba)
    : m_partition(partition)
    , m_lba(lba)
    , m_cursor(0)
{
    uint8_t block_buf[BlockSize];
    _read_lba(partition, lba, block_buf);
    m_size = _file_hdr_of(block_buf).size;
}
// Вычисляет номер блока относительно начала файла для заданного смещения
uint32_t _block_for_offset(uint32_t offset)
{
    if (offset < BlockSize - sizeof(FileHeader)) {
        // В первом блоке (учитываем заголовок)
        return 0;
    }
    // После первого блока работаем с полными блоками
    offset -= (BlockSize - sizeof(FileHeader));
    return 1 + (offset / BlockSize);
}

// Вычисляет смещение внутри блока для заданного смещения в файле
uint32_t _offset_in_block(uint32_t offset)
{
    if (offset < BlockSize - sizeof(FileHeader)) {
        // В первом блоке: данные начинаются с m_data_start
        return sizeof(FileHeader) + offset;
    }
    // После первого блока: данные начинаются с начала блока
    offset -= (BlockSize - sizeof(FileHeader));
    return offset % BlockSize;
}
int File::read(void *buf, size_t n)
{
    if (m_cursor >= m_size || n == 0) {
        return 0;  // EOF или ничего не читаем
    }
    
    // Ограничиваем чтение размером файла
    if (n > m_size - m_cursor) {
        n = m_size - m_cursor;
    }
    
    uint8_t *dest = static_cast<uint8_t*>(buf);
    size_t total_read = 0;
    size_t remaining = n;
    
    while (remaining > 0) {
        // Текущая позиция в файле
        uint32_t current_offset = m_cursor + total_read;
        
        // Определяем текущий блок и смещение в нём
        uint32_t block_idx = _block_for_offset(current_offset);
        uint32_t offset_in_block = _offset_in_block(current_offset);
        uint32_t lba = m_lba + block_idx;
        
        // Сколько можно прочитать из текущего блока
        size_t available_in_block;
        if (block_idx == 0) {
            // Первый блок: данные только после заголовка
            available_in_block = BlockSize - offset_in_block;
        } else {
            // Остальные блоки: полные блоки
            available_in_block = BlockSize - (offset_in_block % BlockSize);
        }
        
        size_t to_read = (remaining < available_in_block) 
                        ? remaining 
                        : available_in_block;
        
        if (offset_in_block == 0 && to_read == BlockSize) {
            // Идеальный случай: читаем полный блок прямо в буфер
            if (_read_lba(m_partition, lba, dest + total_read, 1) < 0) {
                return -1;
            }
        } else {
            // Невыровненное чтение: читаем блок во временный буфер
            uint8_t temp_block[BlockSize];
            if (_read_lba(m_partition, lba, temp_block, 1) < 0) {
                return -1;
            }
            // Копируем нужную часть
            copy_memory_tml(dest+total_read, temp_block+offset_in_block, to_read);
        }
        
        total_read += to_read;
        remaining -= to_read;
    }
    
    m_cursor += total_read;
    return static_cast<int>(total_read);
}
int File::seek(int offset, SeekFrom whence)
{
    uint32_t new_cursor;
    
    switch (whence) {
    case SeekFrom::Begin:
        new_cursor = offset;
        break;
    case SeekFrom::CurrentPosition:
        new_cursor = m_cursor + offset;
        break;
    case SeekFrom::End:
        new_cursor = m_size + offset;
        break;
    default:
        return -1;
    }
    
    // Проверяем границы
    if (new_cursor > m_size) {
        return -1;  // Выход за пределы файла
    }
    
    m_cursor = new_cursor;
    return 0;
}
// int File::read(void *buf, size_t n)
// {
//     if (n == 0 || m_cursor >= m_size) 
//         return 0;

//     const size_t to_read = min(m_size - (size_t)m_cursor, n);
//     size_t remaining = to_read;
//     auto dst = static_cast<uint8_t *>(buf);

//     size_t current_lba = m_lba + (m_cursor + sizeof(FileHeader)) / BlockSize;
//     size_t offset = (m_cursor + sizeof(FileHeader)) % BlockSize;
    
//     if(offset > 0)
//     {
//         uint8_t block[BlockSize];

//         int read_ret = _read_lba(m_partition, current_lba++, block);
//         if(read_ret < 0)
//             return -1;
//         if(read_ret < BlockSize)
//             return 0;
        
//         size_t chunk = min(remaining, BlockSize - offset);
//         copy_memory_tml(dst, block + offset, chunk);

//         dst += chunk;
//         m_cursor += chunk;
//         remaining -= chunk;
//     }

//     if(remaining >= BlockSize)
//     {
//         int read_ret = _read_lba(m_partition, current_lba, dst, remaining / BlockSize);
//         if(read_ret < 0)
//             return read_ret;
//         size_t chunk = remaining / BlockSize * BlockSize;
//         if(read_ret < chunk)
//             return to_read - remaining;

//         current_lba += remaining / BlockSize;
//         dst += chunk;
//         m_cursor += chunk;
//         remaining -= chunk;
//     }

//     if(remaining > 0)
//     {
//         uint8_t block[BlockSize];
//         int read_ret = _read_lba(m_partition, current_lba, dst);
//         if(read_ret < 0)
//             return read_ret;
//         if(read_ret < remaining)
//             return to_read - remaining;

//         m_cursor += remaining;
//     }

//     return n;

//     // const size_t available = m_size - m_cursor;
//     // const size_t to_read = (n > available) ? available : n;
//     // size_t remaining = to_read;
    
//     // uint8_t *dst = static_cast<uint8_t *>(buf);
//     // uint32_t current_lba = m_lba + (m_cursor + sizeof(FileHeader)) / BlockSize;
    
//     // if (size_t offset = (m_cursor + sizeof(FileHeader)) % BlockSize; offset > 0) 
//     // {
//     //     uint8_t block[BlockSize];
        
//     //     if (!_read_lba(m_partition, current_lba++, block))
//     //         return -1;
        
//     //     size_t chunk = min(remaining, BlockSize - offset);
//     //     copy_memory_tml(dst, block + offset, chunk);
        
//     //     dst += chunk;
//     //     remaining -= chunk;
//     //     m_cursor += chunk;
//     // }
    
//     // while (remaining >= BlockSize) 
//     // {
//     //     if(!_read_lba(m_partition, current_lba++, dst)) 
//     //     {
//     //         size_t read_so_far = to_read - remaining;
//     //         m_cursor += read_so_far;
//     //         return (read_so_far > 0) ? read_so_far : -1;
//     //     }
        
//     //     dst += BlockSize;
//     //     remaining -= BlockSize;
//     // }
    
//     // if (remaining > 0) 
//     // {
//     //     uint8_t block[BlockSize];
        
//     //     if (!_read_lba(m_partition, current_lba, block)) 
//     //     {
//     //         size_t read_so_far = to_read - remaining;
//     //         m_cursor += read_so_far;
//     //         return (read_so_far > 0) ? read_so_far : -1;
//     //     }
        
//     //     copy_memory_tml(dst, block, remaining);
//     // }

//     // size_t read_so_far = to_read - remaining;
//     // m_cursor += read_so_far;
//     // m_cursor += to_read - remaining;
//     // return (read_so_far > 0) ? read_so_far : -1;
// }


// int File::seek(int offset, SeekFrom whence)
// {
//     if(whence == SeekFrom::Begin)
//         m_cursor = offset;
//     else if(whence == SeekFrom::CurrentPosition)
//         m_cursor += offset;
//     else if(whence == SeekFrom::End)
//         m_cursor = m_size - offset - 1;
    
//     m_cursor = (uint32_t)clamp((int32_t)m_cursor, 0, (int32_t)(m_size - 1));
//     return m_cursor;
// }


Directory::Directory(IFile &partition, size_t lba)
    : m_partition(&partition)
    , m_lba(lba)
{
    uint8_t block_buf[BlockSize];
    m_partition->seek(m_lba * BlockSize, IFile::SeekFrom::Begin);
    m_partition->read(block_buf, BlockSize);

    auto file_hdr = reinterpret_cast<FileHeader *>(block_buf);
    m_records_count = file_hdr->size / 4;
}


IDirectory::Iterator Directory::next(Iterator prev_it) const
{
    if(is_error_iter(prev_it))
        return sizeof(FileHeader) / 4;
    if(!this->is_existing_iterator(prev_it))
        return end_iter();
    return ++prev_it;
}


IDirectory::Iterator Directory::find(const char *name) const
{
    uint32_t dir_buf_u32[BlockSize / 4], file_buf_u32[BlockSize / 4];

    for(size_t block_idx{}, max_blocks{file_size_to_blocks_count(m_records_count * 4)}
        ; block_idx < max_blocks
        ; ++block_idx)
    {
        _read_lba(*m_partition, m_lba + block_idx, dir_buf_u32);
        size_t data_begin = block_idx == 0 ? sizeof(FileHeader) / 4 : 0;

        size_t records_bound = BlockSize / 4;
        if(block_idx == max_blocks - 1)
            records_bound = (m_records_count + sizeof(FileHeader) / 4) % (BlockSize / 4);

        for(size_t record_idx = data_begin; record_idx < records_bound; ++record_idx)
        {
            _read_lba(*m_partition, dir_buf_u32[record_idx], file_buf_u32);
            if(0 == strncmp(
                _file_hdr_of(file_buf_u32).name, name, sizeof(FileHeader::name)
            )) 
                return _make_dirit(block_idx, record_idx);
        }
    }
    return -1;
}


size_t Directory::get_name_length(Iterator iterator) const
{
    uint32_t block_buf_u32[BlockSize / 4];

    _read_lba(*m_partition, _dirit2lba(iterator, m_lba), block_buf_u32);
    uint32_t file_lba = block_buf_u32[_dirit2offset(iterator)];

    _read_lba(*m_partition, file_lba, block_buf_u32);
    return strlen(_file_hdr_of(block_buf_u32).name);
}


size_t Directory::get_name(Iterator iterator, char *buf, size_t size) const
{
    uint32_t block_buf_u32[BlockSize / 4];

    _read_lba(*m_partition, _dirit2lba(iterator, m_lba), block_buf_u32);
    uint32_t file_lba = block_buf_u32[_dirit2offset(iterator)];

    _read_lba(*m_partition, file_lba, block_buf_u32);
    return strlcpy(buf, _file_hdr_of(block_buf_u32).name, min(size, sizeof(FileHeader::name)));
}


IDirectory::NodeType Directory::get_node_type(Iterator iterator) const
{
    uint32_t block_buf_u32[BlockSize / 4];

    _read_lba(*m_partition, _dirit2lba(iterator, m_lba), block_buf_u32);
    uint32_t file_lba = block_buf_u32[_dirit2offset(iterator)];

    _read_lba(*m_partition, file_lba, block_buf_u32);
    return _file_hdr_of(block_buf_u32).type;
}


size_t Directory::get_node_object_size(Iterator it)
{
    auto type = get_node_type(it);
    if(type == NodeType::File)
        return sizeof(File);
    if(type == NodeType::Directory)
        return sizeof(Directory);
    return 0;
}


void *TAR::Directory::open(Iterator it, void *buf)
{
    uint32_t block_buf_u32[BlockSize / 4];

    _read_lba(*m_partition, _dirit2lba(it, m_lba), block_buf_u32);
    uint32_t file_lba = block_buf_u32[_dirit2offset(it)];

    _read_lba(*m_partition, file_lba, block_buf_u32);
    auto type = _file_hdr_of(block_buf_u32).type;

    if(type == NodeType::Directory)
        return new (buf) Directory(*m_partition, file_lba);
    if(type == NodeType::File)
        return new (buf) File(*m_partition, file_lba);
    return nullptr;
}


Variant<Directory, MountError> TAR::mount(IFile &partition)
{
    char block_buf[BlockSize];

    partition.seek(0 * BlockSize, IFile::SeekFrom::Begin);
    if(BlockSize != partition.read(block_buf, BlockSize))
        return MountError::Read;
    
    auto system_hdr = reinterpret_cast<SystemHeader *>(block_buf);

    if(!cmp_memory_tml(system_hdr->signature, Signature, sizeof(Signature)))
        return MountError::Signature;
    
    if(system_hdr->version != Version)
        return MountError::Version;

    // Root directory is at LBA 1
    return Directory(partition, 1);
}
