#ifndef PLANET_FS_CORE_HPP
#define PLANET_FS_CORE_HPP

#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/handle.hpp>
#include <vector>
#include <deque>
#include <chrono>
#include <algorithm>
#include <stdexcept>
#include <typeindex>
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
    // Entry name not a path
    virtual string_type const& name() const = 0;
    // Entry type code
    virtual file_type type() const noexcept = 0;
    // The size of this entry
    virtual size_t size() const noexcept = 0;
    // Inode of this entry
    virtual st_inode const& inode() const = 0;
    virtual void inode(st_inode const&) = 0;
};

class entry_operation;

class file_entry : public fs_entry,
  public std::enable_shared_from_this<file_entry> {
    typedef char value_type;
    typedef std::vector<value_type> buffer_type;
    typedef default_file_op default_op_type;

    std::string name_;
    op_type_code op_type_index_ = typeid(default_op_type);
    st_inode inode_;
    std::vector<value_type> data_;

    friend class entry_operation;

public:
    file_entry(string_type const& name, op_type_code ti, st_inode const& sti)
        : name_(name), op_type_index_(ti), inode_(sti)
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

    void inode(st_inode const& inode)
    {
        inode_ = inode;
    }

    decltype(data_) const& data_buffer() const
    {
        return data_;
    }

    char const *get_data_ptr() const
    {
        return data_.data();
    }

    op_type_code get_op() const
    {
        return op_type_index_;
    }
};

class dentry : public fs_entry {
public:
    typedef shared_ptr<fs_entry> entry_type;
    typedef default_dir_op default_op_type;
private:
    enum {default_vector_size = 512};

    string_type name_;
    op_type_code op_type_index_ = typeid(default_op_type);
    st_inode inode_;
    std::vector<entry_type> entries_;

public:
    dentry(string_type const& name, op_type_code op, st_inode const& sti = {})
        : name_(name), op_type_index_(op), inode_(sti)
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

    void inode(st_inode const& inode)
    {
        inode_ = inode;
    }

    op_type_code get_op() const
    {
        return op_type_index_;
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

class path_manager {
public:
    typedef unsigned int priority_type;
    typedef op_type_code index_type;
    typedef std::function<bool (path_type const&, file_type)> functor_type;
    typedef std::tuple<priority_type, index_type, functor_type> value_type;
    typedef std::vector<value_type> mapper_type;

    enum priority : priority_type {
        max     = std::numeric_limits<priority_type>::max(),
        normal  = max / 2,
        min     = std::numeric_limits<priority_type>::min(),
    };

    template<typename OpType>
    void add_new_type(priority_type priority)
    {
        path2type_.push_back(
            std::make_tuple(
                priority, index_type(typeid(OpType)), OpType::is_matching_path
            )
        );
        std::stable_sort(path2type_.begin(), path2type_.end(),
            [](value_type const& l, value_type const& r) {
                return std::get<0>(l) > std::get<0>(r);
            }
        );
    }

    template<typename OpType>
    void remove_type()
    {
        path2type_.erase(
            std::remove_if(
                path2type_.begin(), path2type_.end(),[]
                (value_type const& t) {
                    return std::get<1>(t) == typeid(OpType);
                }
            ), path2type_.end()
        );
    }

    index_type matching_type(path_type const& path, file_type type) const
    {
        auto it = std::find_if(path2type_.begin(), path2type_.end(),
            [&](value_type const& t) {
                return std::get<2>(t)(path, type);
            });
        if (it == path2type_.end())
            throw std::runtime_error("No matching type found for " + path.string());
        return std::get<1>(*it);
    }

private:
    mapper_type path2type_;
};

class operation_manager {
public:
    typedef op_type_code index_type;
    typedef shared_ptr<entry_operation> op_type;
    typedef std::map<index_type, op_type> map_type;

    template<typename OpType, typename ...Types>
    void add_new_op(Types&& ...args)
    {
        type2op_.insert(
            std::make_pair(index_type(typeid(OpType)),
                std::make_shared<OpType>(std::forward<Types>(args)...)
            )
        );
    }

    template<typename OpType>
    void remove_op()
    {
        type2op_.erase(
            std::remove(
                type2op_.begin(), type2op_.end(), typeid(OpType)
            ),
            type2op_.end()
        );
    }

    op_type matching_op(index_type const& typeindex) const
    {
        return type2op_.at(typeindex);
    }

    op_type operator[](index_type const& typeindex) const
    {
        return matching_op(typeindex);
    }

private:
    map_type type2op_;
};

class core_file_system {
public:

    typedef path_manager::priority priority;

    explicit core_file_system(mode_t);

    ~core_file_system() = default;

    int getattr(path_type const& path, struct stat& stbuf) const;

    int mknod(path_type const& path, mode_t mode, dev_t device);
    int mknod(path_type const& path, mode_t, dev_t, op_type_code);

    int unlink(path_type const& path);

    int mkdir(path_type const& path, mode_t mode);
    int mkdir(path_type const& path, mode_t mode, op_type_code);

    int rmdir(path_type const& path);

    std::vector<std::string> readdir(path_type const& path) const;

    handle_t open(path_type const& path);

    shared_ptr<fs_entry> get_entry_of(path_type const& path) const;

    template<typename OperationType, typename ...Types>
    void install_op(priority p, Types&& ...args)
    {
        path_mgr_.add_new_type<OperationType>(p);
        ops_mgr_.add_new_op<OperationType>(std::forward<Types>(args)...);
    }

private:
    path_manager        path_mgr_;
    operation_manager   ops_mgr_;
    shared_ptr<dentry>  root = detail::shared_null_ptr;

    shared_ptr<fs_entry> get_entry_of__(shared_ptr<dentry> root, path_type const& path) const;
};


}   // namespace planet

#endif  // PLANET_FS_CORE_HPP
