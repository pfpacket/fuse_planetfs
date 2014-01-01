
#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_entry.hpp>
#include <algorithm>

namespace planet {


    decltype(st_inode::atime)& st_inode::last_access_time()
    {
        return atime;
    }

    decltype(st_inode::atime) const& st_inode::last_access_time() const
    {
        return atime;
    }

    decltype(st_inode::mtime)& st_inode::last_modified_time()
    {
        return mtime;
    }

    decltype(st_inode::mtime) const& st_inode::last_modified_time() const
    {
        return mtime;
    }

    decltype(st_inode::ctime)& st_inode::last_stat_changed_time()
    {
        return ctime;
    }

    decltype(st_inode::ctime) const& st_inode::last_stat_changed_time() const
    {
        return ctime;
    }


    //
    // file_entry
    //
    file_entry::file_entry(string_type name, string_type ops_name, st_inode const& sti)
        : name_(std::move(name)), ops_name_(std::move(ops_name)), inode_(sti)
    {
    }

    string_type const& file_entry::name() const
    {
        return name_;
    }

    file_type file_entry::type() const noexcept
    {
        return file_type::regular_file;
    }

    size_t file_entry::size() const noexcept
    {
        return data_.size();
    }

    st_inode const& file_entry::inode() const
    {
        return inode_;
    }

    void file_entry::inode(st_inode const& inode)
    {
        inode_ = inode;
    }

    string_type const& file_entry::ops_name() const
    {
        return ops_name_;
    }

    decltype(file_entry::data_)& file_entry::data()
    {
        return data_;
    }

    decltype(file_entry::data_) const& file_entry::data() const
    {
        return data_;
    }

    char const *file_entry::get_data_ptr() const
    {
        return data_.data();
    }


    //
    // dentry
    //
    dentry::dentry(string_type name, string_type ops_name, st_inode const& sti)
        : name_(std::move(name)), ops_name_(std::move(ops_name)), inode_(sti)
    {
        entries_.reserve(default_vector_size);
    }

    string_type const& dentry::name() const
    {
        return name_;
    }

    file_type dentry::type() const noexcept
    {
        return file_type::directory;
    }

    size_t dentry::size() const noexcept
    {
        return sizeof (dentry);
    }

    st_inode const& dentry::inode() const
    {
        return inode_;
    }

    void dentry::inode(st_inode const& inode)
    {
        inode_ = inode;
    }

    string_type const& dentry::ops_name() const
    {
        return ops_name_;
    }

    decltype(dentry::entries_) const& dentry::entries() const
    {
        return entries_;
    }

    dentry::entry_type dentry::search_entries(string_type const& name) const
    {
        auto it = std::find_if(entries_.begin(), entries_.end(),
            [&name](entry_type const& ent) {
                return name == ent->name();
            });
        return (it == entries_.end() ?
            detail::shared_null_ptr : *it);
    }

    bool dentry::remove_entry(string_type const& name)
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


}   // namespace planet
