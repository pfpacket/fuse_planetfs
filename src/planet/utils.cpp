
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/fs_core.hpp>
#include <fuse.h>

namespace planet {


    shared_ptr<file_entry> file_cast(shared_ptr<fs_entry> entry)
    {
        if (!entry || entry->type() != file_type::regular_file)
            throw bad_file_cast();
        return std::static_pointer_cast<file_entry>(entry);
    }

    shared_ptr<dentry> directory_cast(shared_ptr<fs_entry> entry)
    {
        if (!entry || entry->type() != file_type::directory)
            throw bad_dir_cast();
        return std::static_pointer_cast<dentry>(entry);
    }

    shared_ptr<file_entry> search_file_entry(core_file_system const& root, path_type const& path)
    {
        auto entry = root.get_entry_of(path);
        return (entry && entry->type() == file_type::regular_file ?
            file_cast(entry) : detail::shared_null_ptr);
    }

    shared_ptr<dentry> search_dir_entry(core_file_system const& root, path_type const& path)
    {
        auto entry = root.get_entry_of(path);
        return (entry && entry->type() == file_type::directory ?
            directory_cast(entry) : detail::shared_null_ptr);
    }

    void fill_st_inode(st_inode& inode)
    {
        //inode.uid = ::fuse_get_context()->uid;
        //inode.gid = ::fuse_get_context()->gid;
        inode.uid = ::getuid();
        inode.gid = ::getgid();
    }

    //
    // raii_wrapper
    //
    raii_wrapper::raii_wrapper(raii_wrapper&& r)
    {
        finalizer_ = std::move(r.finalizer_);
    }

    raii_wrapper& raii_wrapper::operator=(raii_wrapper&& r)
    {
        finalizer_ = std::move(r.finalizer_);
        return *this;
    }

    raii_wrapper::~raii_wrapper()
    {
        try {
            finalize();
        } catch (...) {
            // dtor must not throw any exceptions
        }
    }

    raii_wrapper::functor_t const& raii_wrapper::get_finalizer() const
    {
        return finalizer_;
    }

    void raii_wrapper::finalize() const
    {
        if (finalizer_)
            finalizer_();
    }


}   // namespace planet
