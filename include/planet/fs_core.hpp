#ifndef PLANET_FS_CORE_HPP
#define PLANET_FS_CORE_HPP

#include <planet/common.hpp>
#include <vector>
#include <sys/poll.h>
#include <planet/fs_entry.hpp>
#include <planet/fs_ops_type.hpp>
#include <planet/ops_type_db.hpp>
#include <planet/fuse_poller.hpp>

namespace planet {


    class filesystem;
    class core_file_system
        : public std::enable_shared_from_this<core_file_system> {
    public:

        friend class filesystem;
        typedef ops_type_db::priority priority;

        int getattr(path_type const& path, struct stat& stbuf) const;

        int mknod(path_type const& path, mode_t mode, dev_t device = 0);
        int mknod(path_type const& path, mode_t, dev_t, string_type const&);

        int unlink(path_type const& path, bool force = false);

        int mkdir(path_type const& path, mode_t mode);
        int mkdir(path_type const& path, mode_t mode, string_type const&);

        int rmdir(path_type const& path, bool force = false);

        int chmod(path_type const& path, mode_t mode);

        int chown(path_type const& path, uid_t uid, gid_t gid);

        int truncate(path_type const&, off_t);

        int utimens(path_type const&, struct timespec const [2]);

        std::vector<std::string> readdir(path_type const& path) const;

        handle_t open(path_type const& path);

        int read(handle_t, char *, size_t, off_t);

        int write(handle_t, char const *, size_t, off_t);

        int close(handle_t);

        int poll(handle_t, pollmask_t&);

        int poll(path_type const&, struct fuse_file_info *, struct fuse_pollhandle *, unsigned *);

        shared_ptr<fs_entry> get_entry_of(path_type const& path) const;

        void install_ops(priority p, shared_ptr<fs_ops_type> ops);

        template<typename OperationType, typename ...Types>
        void install_ops(priority p, Types&& ...args)
        {
            this->install_ops(
                p, make_shared<OperationType>(std::forward<Types>(args)...)
            );
        }

        void uninstall_ops(string_type const& name);

        void install_module(priority, string_type const&);
        void install_module(priority, string_type const&, std::vector<string_type> const&);

        void uninstall_module(string_type const&);

        void uninstal_all();

        ops_type_db& get_ops_db();
        ops_type_db const& get_ops_db() const;

    private:
        shared_ptr<dentry>      root    = detail::shared_null_ptr;
        weak_ptr<ops_type_db>   ops_db_;

        handle_manager open_handles_;
        std::once_flag invoke_poller_once_;
        fuse_poller poller_;

        core_file_system() = default;
        static shared_ptr<fs_entry> get_entry_of(shared_ptr<dentry> root, path_type const& path);
        void polling();
    };


}   // namespace planet

#endif  // PLANET_FS_CORE_HPP
