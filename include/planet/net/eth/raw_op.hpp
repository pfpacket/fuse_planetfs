#ifndef PLANET_ETH_RAW_OP_HPP
#define PLANET_ETH_RAW_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace eth {


    class raw_op : public entry_op {
    private:
        int fd_;

        static void bind_to_interface(int fd, std::string const& ifname, int protocol);
        static int do_raw_open(int sock_type, int protocol, std::string const& ifname);

    public:
        raw_op(shared_ptr<core_file_system>)
        {
        }

        ~raw_op() noexcept
        {
        }

        int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
        int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
        int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
        int release(shared_ptr<fs_entry> file_ent) override;
    };

    class raw_type : public file_ops_type {
    private:
    public:
        static const string_type type_name;
        raw_type() : file_ops_type(type_name)
        {
        }

        ~raw_type() noexcept
        {
        }

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override;
        int mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t) override;
        int rmnod(shared_ptr<fs_entry>, path_type const&) override;
        bool match_path(path_type const& path, file_type type) override;
    };


}   // namespace eth
}   // namespace net
}   // namespace planet

#endif  // PLANET_ETH_RAW_OP_HPP
