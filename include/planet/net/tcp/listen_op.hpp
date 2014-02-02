#ifndef PLANET_NET_TCP_LISTEN_OP_HPP
#define PLANET_NET_TCP_LISTEN_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/utils.hpp>

namespace planet {
namespace net {
namespace tcp {


    class listen_op : public default_file_op {
    private:
        shared_ptr<core_file_system> fs_root_;
        path_type path_;
        std::atomic<bool> read_once_{false};
    public:
        listen_op(shared_ptr<core_file_system> fs);

        int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
        int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
        int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
        int release(shared_ptr<fs_entry> file_ent) override;
    };

    class listen_type : public file_ops_type {
    public:
        static const string_type type_name;
        listen_type();

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override;
        bool match_path(path_type const& path, file_type type) override;
    };


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_NET_TCP_LISTEN_OP_HPP
