#ifndef PLANET_TCP_INSTALLER_HPP
#define PLANET_TCP_INSTALLER_HPP

#include <planet/common.hpp>
#include <planet/tcp/common.hpp>
#include <planet/tcp/dir_op.hpp>
#include <planet/tcp/clone_op.hpp>
#include <planet/tcp/ctl_op.hpp>
#include <planet/tcp/data_op.hpp>
#include <planet/tcp/address_op.hpp>
#include <planet/tcp/session_dir_op.hpp>
#include <planet/tcp/client_op.hpp>
#include <planet/tcp/server_op.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace tcp {


class installer : public fs_operation {
public:
    installer(core_file_system& fs_root)
    {
        typedef planet::core_file_system::priority priority;
        fs_root.install_op<dir_op>(priority::normal, fs_root);
        fs_root.install_op<clone_op>(priority::normal, fs_root, 0);
        fs_root.install_op<ctl_op>(priority::normal, fs_root);
        fs_root.install_op<data_op>(priority::normal, fs_root);
        fs_root.install_op<remote_op>(priority::normal, fs_root);
        fs_root.install_op<session_dir_op>(priority::normal, fs_root);
        fs_root.install_op<client_op>(priority::normal);
        fs_root.install_op<server_op>(priority::normal, fs_root);
    }

    static bool is_matching_path(path_type const&, file_type)
    {
        return false;
    }
private:
};


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_INSTALLER_HPP
