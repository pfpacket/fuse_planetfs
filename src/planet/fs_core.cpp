
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/utils.hpp>
#include <syslog.h>

namespace planet {


    core_file_system::core_file_system(mode_t root_mode)
    {
        st_inode new_inode;
        new_inode.mode = root_mode | S_IFDIR;
        install_op<default_file_op>(priority::min);
        install_op<default_dir_op>(priority::min);
        root = std::make_shared<dentry>("/", op_type_code(typeid(default_dir_op)), new_inode);
    }

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
        return mknod(path, mode, device, path_mgr_.matching_type(path, file_type::regular_file));
    }

    int core_file_system::mknod(path_type const& path, mode_t mode, dev_t device, op_type_code op_code)
    {
        if (auto parent_dir = search_dir_entry(*this, path.parent_path())) {
            st_inode new_inode;
            new_inode.dev  = device;
            new_inode.mode = mode | S_IFREG;
            auto fentry =
                parent_dir->add_entry<file_entry>(path.filename().string(), op_code, new_inode);
            try {
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
        int ret = 0;
        if (auto parent_dir = search_dir_entry(*this, path.parent_path())) {
            auto fentry = file_cast(parent_dir->search_entries(path.filename().string()));
            ret = ops_mgr_[fentry->get_op()]->rmnod(fentry, path);
            if (ret < 0)
                return ret;
            parent_dir->remove_entry(path.filename().string());
        } else
            throw exception_errno(ENOENT);
        return ret;
    }

    int core_file_system::mkdir(path_type const& path, mode_t mode)
    {
        return mkdir(path, mode, path_mgr_.matching_type(path, file_type::directory));
    }

    int core_file_system::mkdir(path_type const& path, mode_t mode, op_type_code op)
    {
        if (auto parent_dir = search_dir_entry(*this, path.parent_path())) {
            st_inode new_inode;
            new_inode.mode = mode | S_IFDIR;
            auto dir_entry = parent_dir->add_entry<dentry>(path.filename().string(), op, new_inode);
            try {
                ops_mgr_[dir_entry->get_op()]->mknod(dir_entry, path, mode, 0);
            } catch (...) {
                parent_dir->remove_entry(path.filename().string());
                throw;
            }
        } else
            throw exception_errno(ENOENT);
        return 0;
    }

    int core_file_system::rmdir(path_type const& path)
    {
        int ret = 0;
        if (auto parent_dir = search_dir_entry(*this, path.parent_path())) {
            auto dir_entry = directory_cast(parent_dir->search_entries(path.filename().string()));
            ret = ops_mgr_[dir_entry->get_op()]->rmnod(dir_entry, path);
            if (ret < 0)
                return ret;
            parent_dir->remove_entry(path.filename().string());
        } else
            throw exception_errno(ENOENT);
        return ret;
    }

    std::vector<std::string> core_file_system::readdir(path_type const& path) const
    {
        std::vector<std::string> store;
        if (auto dir_ent = search_dir_entry(*this, path))
            for (auto entry : dir_ent->entries())
                store.push_back(entry->name());
        else
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
            new_handle = handle_mgr.register_op(ops_mgr_[fentry->get_op()]->new_instance(), fentry);
            try {
                auto& op_tuple = handle_mgr.get_operation_entry(new_handle);
                int open_ret = std::get<0>(op_tuple)->open(std::get<1>(op_tuple), path);
                if (open_ret < 0)
                    throw exception_errno(-open_ret);
            } catch (...) {
                handle_mgr.unregister_op(new_handle);
                throw;
            }
        } else
            throw exception_errno(ENOENT);
        return new_handle;
    }

    void core_file_system::install_dynamic_module(priority p, string_type const& mod_name)
    {
        using namespace std::placeholders;
        auto new_op = std::make_shared<dyn_module_op>(mod_name, *this);
        auto functor = std::bind(&dyn_module_op::is_matching_path, new_op.get(), _1, _2);
        path_mgr_.add_new_type<dyn_module_op>(p, op_type_code(mod_name), functor);
        ops_mgr_.add_new_op<dyn_module_op>(mod_name, new_op);
    }

    void core_file_system::uninstall_module(string_type const& mod_name)
    {
        path_mgr_.remove_type(op_type_code(mod_name));
        ops_mgr_.remove_op(op_type_code(mod_name));
    }

    shared_ptr<fs_entry> core_file_system::get_entry_of(path_type const& path) const
    {
        return (path == "/" ? root : get_entry_of__(root, path));
    }

    shared_ptr<fs_entry> core_file_system::get_entry_of__(shared_ptr<dentry> root, path_type const& path) const
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
            : get_entry_of__(directory_cast(dir_entry), "/" + p.substr(pos + 1));
    }


}   // namespace planet
