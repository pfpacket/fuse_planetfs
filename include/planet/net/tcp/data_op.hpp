#ifndef PLANET_TCP_DATA_OP_HPP
#define PLANET_TCP_DATA_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace tcp {


    class data_op : public entry_op {
    private:
        shared_ptr<core_file_system> fs_root_;
        int socket_ = -1;

    public:
        data_op(shared_ptr<core_file_system> fs_root)
            : fs_root_(fs_root)
        {
        }

        ~data_op()
        {
        }

        int open(shared_ptr<fs_entry> fs_ent, path_type const& path) override;
        int read(shared_ptr<fs_entry> fs_ent, char *buf, size_t size, off_t offset) override;
        int write(shared_ptr<fs_entry> fs_ent, char const *buf, size_t size, off_t offset) override;
        int release(shared_ptr<fs_entry> fs_ent) override;
        int poll(pollmask_t&) override;
    };

    class data_type : public file_ops_type {
    public:
        data_type() : file_ops_type("planet.net.tcp.data")
        {
        }

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override
        {
            return std::make_shared<data_op>(fs_root);
        }

        int mknod(shared_ptr<fs_entry> fs_ent, path_type const& path, mode_t, dev_t) override
        {
            return 0;
        }

        int rmnod(shared_ptr<fs_entry> fs_ent, path_type const&) override
        {
            return -EPERM;
        }

        bool match_path(path_type const& path, file_type type) override
        {
            return
                type == file_type::regular_file
                && xpv::regex_match(
                    path.string(),
                    path_reg::data
                );

        }
    };


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_DATA_OP_HPP
