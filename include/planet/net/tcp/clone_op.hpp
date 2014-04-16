#ifndef PLANET_TCP_CLONE_OP_HPP
#define PLANET_TCP_CLONE_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace tcp {


    class clone_op : public default_file_op {
    private:
        shared_ptr<core_file_system> fs_root_;
        int current_fd_;
        handle_t ctl_handle_;

    public:
        clone_op(shared_ptr<core_file_system> root, int current)
            : fs_root_(root), current_fd_(current)
        {
        }

        ~clone_op() noexcept
        {
        }

        int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
        int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
        int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
        int release(shared_ptr<fs_entry> file_ent) override;
    };

    class clone_type : public file_ops_type {
    private:
        int current_fd_;
    public:
        static const string_type type_name;
        clone_type(int start_fd = 0)
            : file_ops_type(type_name), current_fd_(start_fd)
        {
        }

        static bool target_ctl_is_connected(string_type const& ctl_path,
            string_type const& request, shared_ptr<core_file_system> fs_root);

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override;

        int mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t, dev_t) override;

        int rmnod(shared_ptr<fs_entry> file_ent, path_type const&) override;

        bool match_path(path_type const& path, file_type type) override;
    };


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_CLONE_OP_HPP
