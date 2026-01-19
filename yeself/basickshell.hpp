#ifndef YESELF_BASICKSHELL_HPP
#define YESELF_BASICKSHELL_HPP

#include "common.hpp"
#include "ifile.hpp"
#include "idirectory.hpp"
#include "stl.hpp"

class CWDStack
{
public:
    static constexpr size_t MaxSize = 8;
    struct StorageInfo
    {
        char *node_names_buf;
        size_t node_name_size;
        uint8_t *objects_buf;
        size_t object_size;
        size_t max_size;
    };
    enum class CDResult {Ok, StackOverflow, NotFound, NotADirectory, InstantiationFailed};

public:
    CWDStack(IDirectory &root, StorageInfo info);
    void print_cwd(IFile &file, bool do_insert_newline) const;
    CDResult change_dir(const char *dir);
    IDirectory &get_cwd();

private:
    IDirectory &m_root;
    StorageInfo m_storage;
    size_t m_size = 0;
};

class Shell
{
public:
    enum class CommandResult {Ok, EOF, NotFound, BadUsage, ExecError};

public:
    Shell(IFile &tty, CWDStack &cwd_stack);
    void print_start_message();
    CommandResult process_command(char *buf, size_t size);

private:
    IFile &m_tty;
    CWDStack &m_cwd_stack;
};

#endif // YESELF_BASICKSHELL_HPP
