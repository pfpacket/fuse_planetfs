
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/fs_core.hpp>
#include <planet/fs_entry.hpp>
#include <planet/fs_ops_type.hpp>
#include <planet/module_ops_type.hpp>

namespace planet {


    int core_file_system::getattr(path_type const& path, struct stat& stbuf) const
    {
        if (auto fs_ent = this->get_entry_of(path)) {
            stbuf.st_dev   = fs_ent->inode().dev;
            stbuf.st_mode  = fs_ent->inode().mode;
            stbuf.st_nlink = fs_ent.use_count();
            stbuf.st_size  = fs_ent->size();
            stbuf.st_atime = st_inode::to_time_t(fs_ent->inode().atime);
            stbuf.st_mtime = st_inode::to_time_t(fs_ent->inode().mtime);
            stbuf.st_ctime = st_inode::to_time_t(fs_ent->inode().ctime);
        } else
            throw_system_error(ENOENT);
        return 0;
    }

    int core_file_system::mknod(path_type const& path, mode_t mode, dev_t device)
    {
        return this->mknod(path, mode, device,
            ops_db_.lock()->get_name_by_path(path, file_type::regular_file)
        );
    }

    int core_file_system::mknod(
        path_type const& path, mode_t mode, dev_t device, string_type const& ops_name)
    {
        if (auto parent_dir = search_dir_entry(*this, path.parent_path())) {
            st_inode new_inode;
            new_inode.dev  = device;
            new_inode.mode = mode | S_IFREG;
            auto fentry =
                parent_dir->add_entry<file_entry>(path.filename().string(), ops_name, new_inode);
            try {
                ops_db_.lock()->
                    get_ops(ops_name)->mknod(fentry, path, mode, device);
            } catch (...) {
                parent_dir->remove_entry(path.filename().string());
                throw;
            }
        } else
            throw_system_error(ENOENT);
        return 0;
    }

    int core_file_system::unlink(path_type const& path, bool force)
    {
        int ret = 0;
        if (auto parent_dir = search_dir_entry(*this, path.parent_path())) {
            auto fentry = file_cast(parent_dir->search_entries(path.filename().string()));
            ret = ops_db_.lock()->get_ops(fentry->ops_name())->rmnod(fentry, path);
            if (ret < 0 && !force)
                return ret;
            parent_dir->remove_entry(path.filename().string());
        } else
            throw_system_error(ENOENT);
        return ret;
    }

    int core_file_system::mkdir(path_type const& path, mode_t mode)
    {
        return this->mkdir(path, mode,
            ops_db_.lock()->get_name_by_path(path, file_type::directory)
        );
    }

    int core_file_system::mkdir(
        path_type const& path, mode_t mode, string_type const& ops_name)
    {
        if (auto parent_dir = search_dir_entry(*this, path.parent_path())) {
            st_inode new_inode;
            new_inode.mode = mode | S_IFDIR;
            auto dir_entry = parent_dir->add_entry<dentry>(path.filename().string(), ops_name, new_inode);
            try {
                ops_db_.lock()->
                    get_ops(ops_name)->mknod(dir_entry, path, mode, 0);
            } catch (...) {
                parent_dir->remove_entry(path.filename().string());
                throw;
            }
        } else
            throw_system_error(ENOENT);
        return 0;
    }

    int core_file_system::rmdir(path_type const& path, bool force)
    {
        int ret = 0;
        if (auto parent_dir = search_dir_entry(*this, path.parent_path())) {
            auto dir_entry = directory_cast(parent_dir->search_entries(path.filename().string()));
            ret = ops_db_.lock()->get_ops(dir_entry->ops_name())->rmnod(dir_entry, path);
            if (ret < 0 && !force)
                return ret;
            parent_dir->remove_entry(path.filename().string());
        } else
            throw_system_error(ENOENT);
        return ret;
    }

    std::vector<std::string> core_file_system::readdir(path_type const& path) const
    {
        std::vector<std::string> store;
        if (auto dir_ent = search_dir_entry(*this, path))
            for (auto entry : dir_ent->entries())
                store.push_back(entry->name());
        else
            throw_system_error(ENOENT);
        return store;
    }

    handle_t core_file_system::open(path_type const& path)
    {
        handle_t new_handle;
        if (auto entry = get_entry_of(path)) {
            if (entry->type() != file_type::regular_file)
                throw_system_error(EISDIR);
            auto fentry = file_cast(entry);
            new_handle = handle_mgr.register_op(
                ops_db_.lock()->get_ops(fentry->ops_name())->create_op(shared_from_this()), fentry);
            try {
                auto& op_tuple = handle_mgr.get_operation_entry(new_handle);
                int open_ret = std::get<0>(op_tuple)->open(std::get<1>(op_tuple), path);
                if (open_ret < 0)
                    throw_system_error(-open_ret);
            } catch (...) {
                handle_mgr.unregister_op(new_handle);
                throw;
            }
        } else
            throw_system_error(ENOENT);
        return new_handle;
    }

    void core_file_system::install_module(priority p, string_type const& mod_name)
    {
        auto mod_ops = make_shared<module_ops_type>(mod_name);
        ops_db_.lock()->register_type(p, mod_ops);
    }

    void core_file_system::uninstall_module(string_type const& mod_name)
    {
        ops_db_.lock()->unregister_type(mod_name);
    }

    shared_ptr<fs_entry> core_file_system::get_entry_of(path_type const& path) const
    {
        return (path == "/" ? root : get_entry_of(root, path));
    }

    shared_ptr<fs_entry> core_file_system::get_entry_of(shared_ptr<dentry> root, path_type const& path)
    {
        if (path.empty() || path.is_relative() || !root)
            throw std::runtime_error{"detect null parent dir or relative path"};
        string_type p = path.string();
        auto pos = p.find_first_of('/', 1);
        if (pos == std::string::npos)
            return root->search_entries(p.substr(1));
        // Get parent directory from filename
        auto dir_entry = root->search_entries(p.substr(1, pos - 1));
        return (!dir_entry || dir_entry->type() != file_type::directory) ? detail::shared_null_ptr
            : get_entry_of(directory_cast(dir_entry), "/" + p.substr(pos + 1));
    }


}   // namespace planet
