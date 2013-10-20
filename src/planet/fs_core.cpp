
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/utils.hpp>
#include <planet/dyn_module_op.hpp>
#include <syslog.h>

namespace planet {


    core_file_system::core_file_system(
        mode_t root_mode, weak_ptr<path_manager> path_mgr, weak_ptr<operation_manager> ops_mgr
    )   :   path_mgr_(path_mgr), ops_mgr_(ops_mgr)
    {
        st_inode new_inode;
        new_inode.mode = root_mode | S_IFDIR;
        root = std::make_shared<dentry>("/", op_type_code::get<default_dir_op>(), new_inode);
    }

    core_file_system::~core_file_system()
    {
        ::syslog(LOG_NOTICE, "core_file_system: dtor: path_mgr: use_count=%ld", path_mgr_.use_count());
        ::syslog(LOG_NOTICE, "core_file_system: dtor: ops_mgr : use_count=%ld", ops_mgr_.use_count());
    }

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
        auto path_mgr = path_mgr_.lock();
        return mknod(path, mode, device, path_mgr->matching_type(path, file_type::regular_file));
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
                auto ops_mgr = ops_mgr_.lock();
                (*ops_mgr)[fentry->get_op()]->mknod(fentry, path, mode, device);
            } catch (...) {
                parent_dir->remove_entry(path.filename().string());
                throw;
            }
        } else
            throw_system_error(ENOENT);
        return 0;
    }

    int core_file_system::unlink(path_type const& path)
    {
        int ret = 0;
        if (auto parent_dir = search_dir_entry(*this, path.parent_path())) {
            auto fentry = file_cast(parent_dir->search_entries(path.filename().string()));
            auto ops_mgr = ops_mgr_.lock();
            ret = (*ops_mgr)[fentry->get_op()]->rmnod(fentry, path);
            if (ret < 0)
                return ret;
            parent_dir->remove_entry(path.filename().string());
        } else
            throw_system_error(ENOENT);
        return ret;
    }

    int core_file_system::mkdir(path_type const& path, mode_t mode)
    {
        auto path_mgr = path_mgr_.lock();
        return mkdir(path, mode, path_mgr->matching_type(path, file_type::directory));
    }

    int core_file_system::mkdir(path_type const& path, mode_t mode, op_type_code op)
    {
        if (auto parent_dir = search_dir_entry(*this, path.parent_path())) {
            st_inode new_inode;
            new_inode.mode = mode | S_IFDIR;
            auto dir_entry = parent_dir->add_entry<dentry>(path.filename().string(), op, new_inode);
            try {
                auto ops_mgr = ops_mgr_.lock();
                (*ops_mgr)[dir_entry->get_op()]->mknod(dir_entry, path, mode, 0);
            } catch (...) {
                parent_dir->remove_entry(path.filename().string());
                throw;
            }
        } else
            throw_system_error(ENOENT);
        return 0;
    }

    int core_file_system::rmdir(path_type const& path)
    {
        int ret = 0;
        if (auto parent_dir = search_dir_entry(*this, path.parent_path())) {
            auto dir_entry = directory_cast(parent_dir->search_entries(path.filename().string()));
            auto ops_mgr = ops_mgr_.lock();
            ret = (*ops_mgr)[dir_entry->get_op()]->rmnod(dir_entry, path);
            if (ret < 0)
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
            auto ops_mgr = ops_mgr_.lock();
            new_handle = handle_mgr.register_op((*ops_mgr)[fentry->get_op()]->new_instance(), fentry);
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
        using namespace std::placeholders;
        auto new_op = std::make_shared<dyn_module_op>(mod_name, this->shared_from_this());
        auto functor = std::bind(&dyn_module_op::is_matching_path, new_op.get(), _1, _2);
        auto path_mgr = path_mgr_.lock();
        auto ops_mgr  = ops_mgr_.lock();
        path_mgr->add_new_type<dyn_module_op>(p, op_type_code(mod_name), functor);
        ops_mgr->add_new_op<dyn_module_op>(mod_name, new_op);
        ops_mgr->matching_op(op_type_code(mod_name))->install(this->shared_from_this());
    }

    void core_file_system::uninstall_module(string_type const& mod_name)
    {
        auto path_mgr = path_mgr_.lock();
        auto ops_mgr  = ops_mgr_.lock();
        // First, do not create new operation of this type
        path_mgr->remove_type(op_type_code(mod_name));
        // Call uninstaller
        ops_mgr->matching_op(op_type_code(mod_name))->uninstall(this->shared_from_this());
        // Remove operations of this type
        ops_mgr->remove_op(op_type_code(mod_name));
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
