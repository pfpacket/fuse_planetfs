#ifndef PLANET_TCP_DIR_OP_HPP
#define PLANET_TCP_DIR_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace tcp {


    class dir_type final : public file_ops_type {
    private:
        shared_ptr<core_file_system> fs_root_;
    public:
        static const string_type type_name;
        dir_type() : file_ops_type(type_name)
        {
        }

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system>) override;

        int install(shared_ptr<core_file_system> fs) override;

        int mknod(shared_ptr<fs_entry>, path_type const& path, mode_t, dev_t) override;

        int rmnod(shared_ptr<fs_entry>, path_type const&) override;

        bool match_path(path_type const& path, file_type type) override;
    };


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_ETH_DIR_OP_HPP
