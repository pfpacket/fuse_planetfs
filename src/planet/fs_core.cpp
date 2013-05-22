
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <syslog.h>

namespace planet {


    int core_file_system::getattr(path_type const& path, struct stat& stbuf) const
    {
        if (auto fs_ent = get_entry_of(path)) {
            stbuf.st_nlink = fs_ent.use_count();
            stbuf.st_atime = st_inode::to_time_t(fs_ent->inode().atime);
            stbuf.st_mtime = st_inode::to_time_t(fs_ent->inode().mtime);
            stbuf.st_ctime = st_inode::to_time_t(fs_ent->inode().ctime);
            stbuf.st_mode  = fs_ent->inode().mode;
            stbuf.st_size  = fs_ent->size();
        } else
            throw exception_errno(ENOENT);
        return 0;
    }

    int core_file_system::mknod(path_type const& path, mode_t mode, dev_t device)
    {
        return mknod(path, mode, device, path_mgr_[path]);
    }

    int core_file_system::mknod(path_type const& path, mode_t mode, dev_t device, op_type_code op_code)
    {
        if (auto parent_dir = directory_cast(get_entry_of(path.parent_path()))) {
            st_inode new_inode;
            new_inode.dev  = device;
            new_inode.mode = mode | S_IFREG;
            parent_dir->add_entry<file_entry>(path.filename().string(), op_code, new_inode);
            try {
                auto fentry = file_cast(get_entry_of(path));
                ops_mgr_[fentry->get_op()]->mknod(fentry, path, mode, device);
            } catch (...) {
                parent_dir->remove_entry(path.filename().string());
                throw;
            }
        } else
            throw exception_errno(ENOENT);
        return 0;
    }

    int core_file_system::unlink(path_type const& path)
    {
        if (auto parent_dir = directory_cast(get_entry_of(path.parent_path()))) {
            auto fentry = file_cast(get_entry_of(path));
            ops_mgr_[fentry->get_op()]->rmnod(fentry, path);
            parent_dir->remove_entry(path.filename().string());
        } else
            throw exception_errno(ENOENT);
        return 0;
    }

    int core_file_system::mkdir(path_type const& path, mode_t mode)
    {
        if (auto parent_dir = directory_cast(get_entry_of(path.parent_path()))) {
            st_inode new_inode;
            new_inode.mode = mode | S_IFDIR;
            parent_dir->add_entry<dentry>(path.filename().string(), new_inode);
        } else
            throw exception_errno(ENOENT);
        return 0;
    }

    std::vector<std::string> core_file_system::readdir(path_type const& path) const
    {
        std::vector<std::string> store;
        if (auto dir_ent = directory_cast(get_entry_of(path))) {
            for (auto entry : dir_ent->entries())
                store.push_back(entry->name());
        } else
            throw exception_errno(ENOENT);
        return store;
    }

    handle_t core_file_system::open(path_type const& path)
    {
        handle_t new_handle;
        if (auto entry = get_entry_of(path)) {
            if (entry->type() != file_type::regular_file)
                throw exception_errno(EISDIR);
            auto fentry = file_cast(entry);
            new_handle = handle_mgr.register_op(fentry, ops_mgr_[fentry->get_op()]->new_instance());
            try {
                auto& op_tuple = handle_mgr.get_operation_entry(new_handle);
                std::get<1>(op_tuple)->open(std::get<0>(op_tuple), path);
            } catch (...) {
                handle_mgr.unregister_op(new_handle);
                throw;
            }
        } else
            throw exception_errno(ENOENT);
        return new_handle;
    }

    shared_ptr<fs_entry> core_file_system::get_entry_of(path_type const& path) const
    {
        return (path == "/" ? root : get_entry_of__(root, path));
    }

    shared_ptr<fs_entry> core_file_system::get_entry_of__(shared_ptr<dentry> root, path_type const& path) const
    {
        if (path.empty() || path.is_relative() || !root)
            throw std::runtime_error{"empty string or invalid root passed"};
        string_type p = path.string();
        auto pos = p.find_first_of('/', 1);
        if (pos == std::string::npos)
            return root->search_entries(p.substr(1));
        // Get parent directory from filename
        auto dir_entry = root->search_entries(p.substr(1, pos - 1));
        if (dir_entry->type() != file_type::directory)
            throw std::runtime_error{"Not directory entry"};
        return get_entry_of__(directory_cast(dir_entry), "/" + p.substr(pos + 1));
    }


}   // namespace planet
