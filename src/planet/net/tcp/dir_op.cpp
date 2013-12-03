
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/net/tcp/dir_op.hpp>
#include <net/if.h>

namespace planet {
namespace net {
namespace tcp {


    shared_ptr<entry_op> dir_type::create_op(shared_ptr<core_file_system>)
    {
        return planet::detail::shared_null_ptr;
    }

    int dir_type::install(shared_ptr<core_file_system> fs)
    {
        fs_root_ = fs;
        return 0;
    }

    int dir_type::mknod(shared_ptr<fs_entry>, path_type const& path, mode_t, dev_t)
    {
        return fs_root_->mknod("/tcp/clone", S_IRUSR | S_IWUSR, 0);
    }

    int dir_type::rmnod(shared_ptr<fs_entry>, path_type const&)
    {
        throw_system_error(EPERM);
        return -EPERM;
    }

    bool dir_type::match_path(path_type const& path, file_type type)
    {
        return type == file_type::directory && path == "/tcp";
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
