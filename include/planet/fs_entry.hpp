#ifndef PLANET_FS_ENTRY_HPP
#define PLANET_FS_ENTRY_HPP

#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/handle.hpp>
#include <vector>
#include <chrono>
#include <algorithm>
#include <sys/stat.h>

namespace planet {


// inode structure
class st_inode {
public:
    dev_t dev = 0;
    mode_t mode = 0;
    std::chrono::system_clock::time_point
        atime = std::chrono::system_clock::now(),
        mtime = std::chrono::system_clock::now(),
        ctime = std::chrono::system_clock::now();

    decltype(atime) last_access_time() const
    {
        return atime;
    }
    decltype(mtime) last_modified_time() const
    {
        return mtime;
    }
    decltype(ctime) last_stat_changed_time() const
    {
        return ctime;
    }
    template<typename Clock, typename Duration>
    static std::time_t to_time_t(std::chrono::time_point<Clock, Duration> const& t)
    {
        return Clock::to_time_t(t);
    }
};


//
// fs_entry abstract base class
//    for every filesystem entry
//
class fs_entry {
public:
    virtual ~fs_entry() = default;
    // Entry name not a path
    virtual string_type const& name() const = 0;
    // Entry type code
    virtual file_type type() const noexcept = 0;
    // The size of this entry
    virtual size_t size() const noexcept = 0;
    // Inode of this entry
    virtual st_inode const& inode() const = 0;
    virtual void inode(st_inode const&) = 0;
    // Operations name
    virtual string_type const& ops_name() const = 0;
};

class file_entry : public fs_entry {
    typedef char value_type;
    typedef std::vector<value_type> buffer_type;
    typedef default_file_op default_op_type;

    std::string name_, ops_name_;
    st_inode inode_;
    std::vector<value_type> data_;

public:
    file_entry(string_type name, string_type ops_name, st_inode const& sti)
        : name_(std::move(name)), ops_name_(std::move(ops_name)), inode_(sti)
    {
    }

    virtual ~file_entry()
    {
    }

    string_type const& name() const override
    {
        return name_;
    }

    file_type type() const noexcept override
    {
        return file_type::regular_file;
    }

    size_t size() const noexcept override
    {
        return data_.size();
    }

    st_inode const& inode() const override
    {
        return inode_;
    }

    void inode(st_inode const& inode) override
    {
        inode_ = inode;
    }

    string_type const& ops_name() const override
    {
        return ops_name_;
    }

    decltype(data_)& data()
    {
        return data_;
    }

    decltype(data_) const& data() const
    {
        return data_;
    }

    char const *get_data_ptr() const
    {
        return data_.data();
    }
};

class dentry : public fs_entry {
public:
    typedef shared_ptr<fs_entry> entry_type;
    typedef default_dir_op default_op_type;
private:
    enum {default_vector_size = 512};

    string_type name_, ops_name_;
    st_inode inode_;
    std::vector<entry_type> entries_;

public:
    dentry(string_type name, string_type ops_name, st_inode const& sti = {})
        : name_(std::move(name)), ops_name_(std::move(ops_name)), inode_(sti)
    {
        entries_.reserve(default_vector_size);
    }

    virtual ~dentry()
    {
    }

    string_type const& name() const override
    {
        return name_;
    }

    file_type type() const noexcept override
    {
        return file_type::directory;
    }

    size_t size() const noexcept override
    {
        return sizeof (dentry);
    }

    st_inode const& inode() const override
    {
        return inode_;
    }

    void inode(st_inode const& inode) override
    {
        inode_ = inode;
    }

    string_type const& ops_name() const override
    {
        return ops_name_;
    }

    decltype(entries_) const& entries() const
    {
        return entries_;
    }

    entry_type search_entries(string_type const& name) const
    {
        auto it = std::find_if(entries_.begin(), entries_.end(),
            [&name](entry_type const& ent) {
                return name == ent->name();
            });
        return (it == entries_.end() ?
            detail::shared_null_ptr : *it);
    }

    template<typename EntryType, typename... Types>
    shared_ptr<EntryType> add_entry(Types&&... args)
    {
        auto entry = std::make_shared<EntryType>(std::forward<Types>(args)...);
        entries_.push_back(entry);
        return entry;
    }

    bool remove_entry(string_type const& name)
    {
        auto it = entries_.erase(
            std::remove_if(
                entries_.begin(), entries_.end(),
                [&name](entry_type const& ent) {
                    return name == ent->name();
                }
            ),
            entries_.end()
        );
        return it != entries_.end();
    }
};


}   // namespace planet

#endif  // PLANET_FS_ENTRY_HPP
