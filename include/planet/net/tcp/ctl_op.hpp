#ifndef PLANET_TCP_CTL_OP_HPP
#define PLANET_TCP_CTL_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace tcp {


    class ctl_op : public entry_op {
    private:
        shared_ptr<core_file_system> fs_root_;
        int current_fd_;
        bool fd_number_already_read_ = false;
        int interpret_request(string_type const& request);

    public:

        //ctl_op() = default;
        ctl_op(shared_ptr<core_file_system> root)
            : fs_root_(root)
        {
        }

        ~ctl_op()
        {
        }

        int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
        int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
        int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
        int release(shared_ptr<fs_entry> file_ent) override;
    };

    class ctl_type : public file_ops_type {
    public:
        static const string_type type_name;
        ctl_type() : file_ops_type(type_name)
        {
        }

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root)
        {
            return std::make_shared<ctl_op>(fs_root);
        }

        int mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t, dev_t)
        {
            return 0;
        }

        int rmnod(shared_ptr<fs_entry> file_ent, path_type const&)
        {
            return -EPERM;
        }

        bool match_path(path_type const& path, file_type type)
        {
            return
                type == file_type::regular_file
                && xpv::regex_match(
                    path.string(),
                    path_reg::ctl
                );
        }
    };


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_CTL_OP_HPP
