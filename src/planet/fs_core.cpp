
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/utils.hpp>
#include <planet/module_ops_type.hpp>

namespace planet {


    int core_file_system::getattr(path_type const& path, struct stat& stbuf) const
    {
        if (auto fs_ent = this->get_entry_of(path)) {
            stbuf.st_dev    = fs_ent->inode().dev;
            stbuf.st_mode   = fs_ent->inode().mode;
            stbuf.st_uid    = fs_ent->inode().uid;
            stbuf.st_gid    = fs_ent->inode().gid;
            stbuf.st_nlink  = fs_ent.use_count() - 1;
            stbuf.st_size   = fs_ent->size();
            stbuf.st_atime  = st_inode::to_time_t(fs_ent->inode().atime);
            stbuf.st_mtime  = st_inode::to_time_t(fs_ent->inode().mtime);
            stbuf.st_ctime  = st_inode::to_time_t(fs_ent->inode().ctime);
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
            fill_st_inode(new_inode);
            new_inode.dev  = device;
            new_inode.mode = mode | S_IFREG;
            auto fentry =
                parent_dir->add_entry<file_entry>(path.filename().string(), ops_name, new_inode);
            try {
                ::syslog(LOG_NOTICE, "mknod: Creating \'%s\' type=%s",
                    path.string().data(), ops_name.data());
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
            fill_st_inode(new_inode);
            new_inode.mode = mode | S_IFDIR;
            auto dir_entry = parent_dir->add_entry<dentry>(path.filename().string(), ops_name, new_inode);
            try {
                ::syslog(LOG_NOTICE, "mkdir: Creating \'%s\' type=%s",
                    path.string().data(), ops_name.data());
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

    int core_file_system::chmod(path_type const& path, mode_t mode)
    {
        int ret = 0;
        if (auto entry = this->get_entry_of(path)) {
            auto new_inode = entry->inode();
            new_inode.mode = mode;
            new_inode.ctime = std::chrono::system_clock::now();
            entry->inode(new_inode);
        } else
            ret = -ENOENT;
        return ret;
    }

    int core_file_system::chown(path_type const& path, uid_t uid, gid_t gid)
    {
        int ret = 0;
        if (auto entry = this->get_entry_of(path)) {
            auto new_inode = entry->inode();
            new_inode.uid = uid;
            new_inode.gid = gid;
            entry->inode(new_inode);
        } else
            ret = -ENOENT;
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
            new_handle = open_handles_.register_op(
                ops_db_.lock()->get_ops(fentry->ops_name())->create_op(shared_from_this()), fentry);
            try {
                auto& op_tuple = open_handles_.get_op_entry(new_handle);
                int open_ret = std::get<0>(op_tuple)->open(std::get<1>(op_tuple), path);
                if (open_ret < 0)
                    throw_system_error(-open_ret);
            } catch (...) {
                open_handles_.unregister_op(new_handle);
                throw;
            }
        } else
            throw_system_error(ENOENT);
        return new_handle;
    }

    int core_file_system::read(handle_t handle, char *buf, size_t size, off_t offset)
    {
        auto& op_tuple = open_handles_.get_op_entry(handle);
        return std::get<0>(op_tuple)->read(
            std::get<1>(op_tuple), buf, size, offset
        );
    }

    int core_file_system::write(handle_t handle, char const *buf, size_t size, off_t offset)
    {
        auto& op_tuple = open_handles_.get_op_entry(handle);
        return std::get<0>(op_tuple)->write(
            std::get<1>(op_tuple), buf, size, offset
        );
    }

    int core_file_system::close(handle_t handle)
    {
        auto& op_tuple = open_handles_.get_op_entry(handle);
        raii_wrapper raii([this, handle] {
            raii_wrapper raii([this,handle] {
                this->open_handles_.unregister_op(handle);
            });
            this->poller_.unregister(handle);
        });
        int ret = std::get<0>(op_tuple)->release(std::get<1>(op_tuple));
        return ret;
    }

    int core_file_system::poll(handle_t handle, pollmask_t& pollmask)
    {
        auto& op_tuple = open_handles_.get_op_entry(handle);
        return std::get<0>(op_tuple)->poll(pollmask);
    }

    int core_file_system::poll(
        path_type const& path, struct fuse_file_info *fi, struct fuse_pollhandle *ph, unsigned *reventsp
    )
    {
        auto handle = get_handle_from(*fi);
        std::call_once(invoke_poller_once_, [this]() {
            this->poller_.poll(this->shared_from_this());
        });

        if (ph) {
            //syslog_fmt(LOG_NOTICE, format("%s: handle=%d adding new handle") % __func__ % handle);
            poller_.register_new(handle, ph);
        }

        *reventsp |= poller_.get_status(handle);
        //syslog_fmt(LOG_NOTICE, format("%1%: handle=%2% polled=%3%") % __func__ % handle % *reventsp);
        return 0;

    }

    void core_file_system::install_ops(priority p, shared_ptr<fs_ops_type> ops)
    {
        ::syslog(LOG_NOTICE, "Installing ops: %s", ops->name().c_str());
        ops_db_.lock()->register_ops(p, ops);
    }

    void core_file_system::uninstall_ops(string_type const& name)
    {
        ::syslog(LOG_NOTICE, "Uninstalling ops: %s", name.c_str());
        ops_db_.lock()->unregister_ops(name);
    }

    void core_file_system::install_module(priority p, string_type const& mod_name)
    {
        ::syslog(LOG_NOTICE, "Installing module: %s", mod_name.c_str());
        ops_db_.lock()->register_ops(p, make_shared<module_ops_type>(mod_name));
    }

    void core_file_system::install_module(
        priority p, string_type const& mod_name, std::vector<string_type> const& paths)
    {
        ::syslog(LOG_NOTICE, "Installing module: %s", mod_name.c_str());
        ops_db_.lock()->register_ops(
            p, make_shared<module_ops_type>(mod_name, paths)
        );
    }

    void core_file_system::uninstall_module(string_type const& mod_name)
    {
        ::syslog(LOG_NOTICE, "Uninstalling module: %s", mod_name.c_str());
        ops_db_.lock()->unregister_ops(mod_name);
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

    ops_type_db& core_file_system::get_ops_db()
    {
        auto db = ops_db_.lock();
        if (!db)
            throw std::runtime_error("get_ops_db(): ops_db_ not available");
        return *db;
    }

    ops_type_db const& core_file_system::get_ops_db() const
    {
        auto db = ops_db_.lock();
        if (!db)
            throw std::runtime_error("get_ops_db(): ops_db_ not available");
        return *db;
    }


}   // namespace planet
