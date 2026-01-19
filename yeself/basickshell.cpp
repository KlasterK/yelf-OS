#include "basickshell.hpp"
#include "fileops.hpp"
#include "stringops.hpp"
#include "logging.hpp"

CWDStack::CWDStack(IDirectory &root, StorageInfo info)
    : m_root(root)
    , m_storage(info)
{}

void CWDStack::print_cwd(IFile &file, bool do_insert_newline) const
{
    if(m_size == 0)
    {
        putc(file, '/');
        if(do_insert_newline)
            putc(file, '\n');
        return;
    }

    for(size_t i{}; i < m_size; ++i)
    {
        putc(file, '/');
        puts(file, m_storage.node_names_buf + m_storage.node_name_size * i);
    }

    if(do_insert_newline)
        putc(file, '\n');
}

CWDStack::CDResult CWDStack::change_dir(const char *dir)
{
    if(m_size >= m_storage.max_size)
        return CDResult::StackOverflow;

    if(0 == strcmp(dir, ".."))
    {
        if(m_size == 0)
            return CDResult::Ok;
        
        void *entry = m_storage.objects_buf + m_storage.object_size * --m_size;
        static_cast<IDirectory *>(entry)->~IDirectory();
        return CDResult::Ok;
    }
    
    if(0 == strcmp(dir, "."))
        return CDResult::Ok;

    IDirectory *parent = &m_root;
    if(m_size > 0)
    {
        void *entry = m_storage.objects_buf + m_storage.object_size * (m_size - 1);
        parent = static_cast<IDirectory *>(entry);
    }

    auto iter = parent->find(dir);
    if(IDirectory::is_error_iter(iter))
        return CDResult::NotFound;
    if(parent->get_node_type(iter) != IDirectory::NodeType::Directory)
        return CDResult::NotADirectory;

    void *child_buf = m_storage.objects_buf + m_storage.object_size * m_size;

    if(nullptr == parent->open(iter, child_buf))
        return CDResult::InstantiationFailed;

    parent->get_name(
        iter, 
        m_storage.node_names_buf + m_storage.node_name_size * m_size, 
        m_storage.node_name_size
    );

    ++m_size;
    return CDResult::Ok;
}

IDirectory &CWDStack::get_cwd()
{
    if(m_size == 0)
        return m_root;

    void *entry = m_storage.objects_buf + m_storage.object_size * (m_size - 1);
    return *static_cast<IDirectory *>(entry);
}



Shell::Shell(IFile &tty, CWDStack &cwd_stack)
    : m_tty(tty)
    , m_cwd_stack(cwd_stack)
{}

void Shell::print_start_message()
{
    puts(m_tty, "Operations: cd pwd ls cat help more\n");
}

Shell::CommandResult Shell::process_command(char *buf, size_t size)
{
    m_cwd_stack.print_cwd(m_tty, false);
    puts(m_tty, " $ ");
    if(nullptr == readline(m_tty, buf, size))
        return CommandResult::EOF;
    
    auto [command, input_size] = string_strip(buf);
    const char *argument = nullptr;
    for(size_t i{}; command[i] != 0; ++i)
    {
        if(is_whitespace(command[i]))
        {
            command[i] = 0;
            for(argument = command + i + 1; is_whitespace(*argument); ++argument) {}
            break;
        }
    }

    if(0 == strcmp("cd", command))
    {
        if(argument == nullptr)
        {
            puts(m_tty, "usage: cd <dir>\n");
            return CommandResult::BadUsage;
        }

        switch(m_cwd_stack.change_dir(argument))
        {
            using enum CWDStack::CDResult;
        case Ok:
            return CommandResult::Ok;
        case StackOverflow:
            puts(m_tty, "Sorry but you cannot forward more, because the stack limit is over.\n");
            return CommandResult::ExecError;
        case NotFound:
            printf(m_tty, "Directory '%s' not found.\n", argument);
            return CommandResult::ExecError;
        case NotADirectory:
            puts(m_tty, "Object is not a directory.\n");
            return CommandResult::ExecError;
        case InstantiationFailed:
            puts(m_tty, "Object cannot be instantiated. Please contact devs.\n");
            return CommandResult::ExecError;
        }
    }

    if(0 == strcmp("pwd", command))
    {
        m_cwd_stack.print_cwd(m_tty, true);
        return CommandResult::Ok;
    }

    if(0 == strcmp("ls", command))
    {
        auto iter = IDirectory::before_begin_iter();
        auto &cwd = m_cwd_stack.get_cwd();
        for(;;)
        {
            iter = cwd.next(iter);
            if(IDirectory::is_error_iter(iter))
                break;
            
            cwd.get_name(iter, buf, size);

            const char *type = "Unknown"; 
            switch(cwd.get_node_type(iter))
            {
            case IDirectory::NodeType::Directory:
                type = "Directory";
                break;
            case IDirectory::NodeType::File:
                type = "File";
                break;
            default:
            }
            
            printf(m_tty, "%s - %s (%x)\n", type, buf, iter);
        }
        return CommandResult::Ok;
    }

    if(0 == strcmp("cat", command))
    {
        if(argument == nullptr)
        {
            puts(m_tty, "usage: cat <file>\n");
            return CommandResult::BadUsage;
        }

        auto &cwd = m_cwd_stack.get_cwd();
        auto iter = cwd.find(argument);

        if(IDirectory::is_error_iter(iter))
        {
            puts(m_tty, "File not found.\n");
            return CommandResult::ExecError;
        }

        if(cwd.get_node_type(iter) != IDirectory::NodeType::File)
        {
            puts(m_tty, "Object is not a file.\n");
            return CommandResult::ExecError;
        }

        alignas(16) uint8_t file_raw_storage[cwd.get_node_object_size(iter)];
        auto file = static_cast<IFile *>(cwd.open(iter, file_raw_storage));
        if(file == nullptr)
        {
            puts(m_tty, "Object cannot be instantiated. Please contact devs.\n");
            return CommandResult::ExecError;
        }

        size_t nread{};
        for(;;)
        {
            nread = file->read(buf, size);
            if(nread < 1)
                break;
            
            m_tty.write(buf, nread);
        }
        
        if(nread >= 1 && buf[nread-1] != '\n')
            puts(m_tty, "\n(no newline)\n");

        file->~IFile();
        return CommandResult::Ok;
    }

    if(0 == strcmp("help", command))
    {
        print_start_message();
        return CommandResult::Ok;
    }

    if(0 == strcmp("more", command))
    {
        if(m_tty.ioctl(IOCtlFunctions::IsTerminal, nullptr) < 0)
        {
            puts(m_tty, "Pager cannot be used out of terminal.\n");
            return CommandResult::ExecError;
        }

        if(argument == nullptr)
        {
            puts(m_tty, "usage: more <file>\n");
            return CommandResult::BadUsage;
        }

        auto &cwd = m_cwd_stack.get_cwd();
        auto iter = cwd.find(argument);

        if(IDirectory::is_error_iter(iter))
        {
            puts(m_tty, "File not found.\n");
            return CommandResult::ExecError;
        }

        if(cwd.get_node_type(iter) != IDirectory::NodeType::File)
        {
            puts(m_tty, "Object is not a file.\n");
            return CommandResult::ExecError;
        }

        alignas(16) uint8_t file_raw_storage[cwd.get_node_object_size(iter)];
        auto file = static_cast<IFile *>(cwd.open(iter, file_raw_storage));
        if(file == nullptr)
        {
            puts(m_tty, "Object cannot be instantiated. Please contact devs.\n");
            return CommandResult::ExecError;
        }

        // size_t nread{};
        // for(;;)
        // {
        //     nread = file->read(buf, size);
        //     if(nread < 1)
        //         break;
            
        //     m_tty.write(buf, nread);
        // }
        
        // if(nread >= 1 && buf[nread-1] != '\n')
        //     puts(m_tty, "\n(no newline)\n");

        int height = m_tty.ioctl(IOCtlFunctions::GetTerminalHeight, nullptr);
        putc(m_tty, '\f');
        for(;;)
        {
            int c = getc(*file);
            if(c < 0)
                break;

            putc(m_tty, c);
            if(m_tty.ioctl(IOCtlFunctions::GetLineCounter, nullptr) == height - 1)
            {
                constexpr const char *bs32 = "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
                constexpr const char *sp32 = "                                ";

                puts(m_tty, "\b\nPress any key to continue . . . ");
                getc(m_tty);
                puts(m_tty, bs32);
                puts(m_tty, sp32);
                puts(m_tty, bs32);
                putc(m_tty, '\f');
            }
        }

        file->~IFile();
        return CommandResult::Ok;
    }

    if(command[0] != 0)
    {
        printf(m_tty, "Unknown operation '%s'.\n", command);
        return CommandResult::Ok;
    }

    return CommandResult::Ok;
}
