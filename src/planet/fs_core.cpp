
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/utils.hpp>
#include <planet/module_ops_type.hpp>

namespace planet {


    static void update_last_access_time(shared_ptr<fs_entry> const& entry)
    {
        auto new_inode = entry->inode();
        new_inode.atime = std::chrono::system_clock::now();
        entry->inode(new_inode);
    }

    static void update_last_modified_time(shared_ptr<fs_entry> const& entry)
    {
        auto new_inode = entry->inode();
        new_inode.mtime = std::chrono::system_clock::now();
        entry->inode(new_inode);
    }

    static void update_last_stat_change_time(shared_ptr<fs_entry> const& entry)
    {
        auto new_inode = entry->inode();
        new_inode.ctime = std::chrono::system_clock::now();
        entry->inode(new_inode);
    }

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
                BOOST_LOG_TRIVIAL(info) << "mknod: Creating \'" << path << "\' type=" << ops_name;
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
                BOOST_LOG_TRIVIAL(info) << "mkdir: Creating \'" << path << "\' type=" << ops_name;
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
            new_inode.ctime = std::chrono::system_clock::now();
            entry->inode(new_inode);
        } else
            ret = -ENOENT;
        return ret;
    }

    int core_file_system::truncate(path_type const& path, off_t offset)
    {
        int ret = 0;
        if (auto entry = this->get_entry_of(path))
            planet::file_cast(entry)->data().resize(offset);
        else
            ret = -ENOENT;
        return ret;
    }

    int core_file_system::utimens(path_type const& path, struct timespec const tv[2])
    {
        int ret = 0;
        namespace ch = std::chrono;
        if (auto entry = this->get_entry_of(path)) {
            ch::nanoseconds nano_access(tv[0].tv_nsec), nano_mod(tv[1].tv_nsec);
            ch::seconds sec_access(tv[0].tv_sec), sec_mod(tv[1].tv_sec);
            planet::st_inode new_inode = entry->inode();
            new_inode.atime = decltype(new_inode.atime)
                (ch::duration_cast<decltype(new_inode.atime)::duration>(nano_access + sec_access));
            new_inode.mtime = decltype(new_inode.atime)
                (ch::duration_cast<decltype(new_inode.atime)::duration>(nano_mod + sec_mod));
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
                update_last_access_time(fentry);
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
        int ret = std::get<0>(op_tuple)->read(
            std::get<1>(op_tuple), buf, size, offset
        );
        update_last_access_time(std::get<1>(op_tuple));
        return ret;
    }

    int core_file_system::write(handle_t handle, char const *buf, size_t size, off_t offset)
    {
        auto& op_tuple = open_handles_.get_op_entry(handle);
        int ret = std::get<0>(op_tuple)->write(
            std::get<1>(op_tuple), buf, size, offset
        );
        update_last_access_time(std::get<1>(op_tuple));
        update_last_modified_time(std::get<1>(op_tuple));
        return ret;
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
            BOOST_LOG_TRIVIAL(trace) << __func__ << ": handle=" << handle << " adding new handle";
            poller_.register_new(handle, ph);
        }

        *reventsp |= poller_.get_status(handle);
        BOOST_LOG_TRIVIAL(trace) << __func__ << ": handle=" << handle << " polled=" << *reventsp;
        return 0;

    }

    void core_file_system::install_ops(priority p, shared_ptr<fs_ops_type> ops)
    {
        BOOST_LOG_TRIVIAL(info) << "Installing ops: " << ops->name().c_str();
        raii_wrapper raii([this, ops] {
            if (std::uncaught_exception())
                this->ops_db_.lock()->unregister_ops(ops->name());
        });
        ops_db_.lock()->register_ops(p, ops);
        int ret = ops->install(this->shared_from_this());
        if (ret < 0)
            throw_system_error(-ret, ops->name() + ": registering ops failed");
    }

    void core_file_system::uninstall_ops(string_type const& name)
    {
        BOOST_LOG_TRIVIAL(info) << "Uninstalling ops: " << name;
        raii_wrapper finalize([this, &name]() {
            ops_db_.lock()->unregister_ops(name);
        });
        ops_db_.lock()->get_ops(name)->uninstall(this->shared_from_this());
    }

    void core_file_system::install_module(priority p, string_type const& mod_name)
    {
        this->install_ops(p, make_shared<module_ops_type>(mod_name));
    }

    void core_file_system::install_module(
        priority p, string_type const& mod_name, std::vector<string_type> const& paths)
    {
        this->install_ops(p, make_shared<module_ops_type>(mod_name, paths));
    }

    void core_file_system::uninstall_module(string_type const& mod_name)
    {
        BOOST_LOG_TRIVIAL(info) << "Uninstalling module: " << mod_name;
        this->uninstall_ops(mod_name);
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

    void core_file_system::uninstal_all()
    {
        auto db = ops_db_.lock();
        for (auto&& info : db->info())
            db->get_ops(std::get<0>(info))->uninstall(this->shared_from_this());
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
