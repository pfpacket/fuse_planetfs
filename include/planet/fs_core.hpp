#ifndef PLANET_FS_CORE_HPP
#define PLANET_FS_CORE_HPP

#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/planet_handle.hpp>
#include <vector>
#include <deque>
#include <string>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <sys/stat.h>

namespace planet {


// inode structure
class st_inode {
public:
    dev_t dev = 0;
    mode_t mode = 0;
    time_t atime = std::time(nullptr),
           mtime = std::time(nullptr),
           ctime = std::time(nullptr);

    time_t get_last_access_time() const
    {
        return atime;
    }
    time_t get_last_modified_time() const
    {
        return mtime;
    }
    time_t get_last_status_changed() const
    {
        return ctime;
    }
};


//
// fs_entry abstract base class
//    for every filesystem entry
//
class fs_entry {
public:
    // Entry name not a path
    virtual string_type const& name() const = 0;
    // Entry type code
    virtual file_type type() const noexcept = 0;
    // The size of this entry
    virtual size_t size() const noexcept = 0;
    // Inode of this entry
    virtual st_inode const& inode() const = 0;
};

class planet_operation;

class file_entry : public fs_entry,
  public std::enable_shared_from_this<file_entry> {
    typedef char value_type;
    typedef std::vector<value_type> buffer_type;

    std::string name_;
    opcode op_;
    st_inode inode_;
    std::vector<value_type> data_;

    friend class planet_operation;

public:
    file_entry(string_type const& name, opcode op, st_inode const& sti)
        : name_(name), op_(op), inode_(sti)
    {
    }

    string_type const& name() const
    {
        return name_;
    }

    file_type type() const noexcept
    {
        return file_type::regular_file;
    }

    size_t size() const noexcept
    {
        return data_.size();
    }

    st_inode const& inode() const
    {
        return inode_;
    }

    decltype(data_) const& data_buffer() const
    {
        return data_;
    }

    char const *get_data_ptr() const
    {
        return data_.data();
    }

    opcode get_op() const
    {
        return op_;
    }
};

class dentry : public fs_entry {
public:
    typedef shared_ptr<fs_entry> entry_type;
private:
    enum {default_vector_size = 512};

    string_type name_;
    st_inode inode_;
    std::vector<entry_type> entries_;

public:
    dentry(string_type const& name, st_inode const& sti = {})
        : name_(name), inode_(sti)
    {
        entries_.reserve(default_vector_size);
    }

    virtual ~dentry() noexcept
    {
    }

    string_type const& name() const
    {
        return name_;
    }

    file_type type() const noexcept
    {
        return file_type::directory;
    }

    size_t size() const noexcept
    {
        return sizeof (dentry);
    }

    st_inode const& inode() const
    {
        return inode_;
    }

    decltype(entries_) const& entries() const
    {
        return entries_;
    }

    shared_ptr<fs_entry> search_entries(string_type const& name) const
    {
        auto it = std::find_if(entries_.begin(), entries_.end(),
            [&name](entry_type const& ent) {
                return name == ent->name();
            });
        return (it == entries_.end() ?
            detail::shared_null_ptr : *it);
    }

    template<typename EntryType, typename... Types>
    bool add_entry(Types&&... args)
    {
        entries_.push_back(std::make_shared<EntryType>(std::forward<Types>(args)...));
        return true;
    }

    bool remove_entry(string_type const& name)
    {
        entries_.erase(
            std::remove_if(
                entries_.begin(), entries_.end(),
                [&name](entry_type const& ent) {
                    return name == ent->name();
                }
            ),
            entries_.end()
        );
        return true;
    }
};

class core_file_system {
public:
    core_file_system() = default;
    ~core_file_system() = default;

    int getattr(path_type const& path, struct stat& stbuf) const;

    int mknod(path_type const& path, mode_t mode, dev_t device);

    int unlink(path_type const& path);

    int mkdir(path_type const& path, mode_t mode);

    int readdir(path_type const& path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info& fi);

    handle_t open(path_type const& path);

    shared_ptr<fs_entry> get_entry_of(path_type const& path) const;

    template<typename OperationType, typename ...Types>
    void install_op(Types&& ...args)
    {
        auto installer
            = [] { return std::make_shared<OperationType>(); };
    }

private:
    shared_ptr<dentry> root = std::make_shared<dentry>("/");
    
    shared_ptr<fs_entry> get_entry_of__(shared_ptr<dentry> root, path_type const& path) const;
};


}   // namespace planet

#endif  // PLANET_FS_CORE_HPP
