#ifndef PLANET_TCP_INSTALLER_HPP
#define PLANET_TCP_INSTALLER_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/net/tcp/dir_op.hpp>
#include <planet/net/tcp/clone_op.hpp>
#include <planet/net/tcp/ctl_op.hpp>
#include <planet/net/tcp/data_op.hpp>
#include <planet/net/tcp/address_op.hpp>
#include <planet/net/tcp/session_dir_op.hpp>
#include <planet/net/tcp/client_op.hpp>
#include <planet/net/tcp/server_op.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace tcp {


class installer : public fs_operation {
private:
    core_file_system& fs_root_;
public:
    installer(core_file_system& fs_root) : fs_root_(fs_root)
    {
        typedef planet::core_file_system::priority priority;
        fs_root.install_op<dir_op>(priority::normal, fs_root);
        fs_root.install_op<clone_op>(priority::normal, fs_root, 0);
        fs_root.install_op<ctl_op>(priority::normal, fs_root);
        fs_root.install_op<data_op>(priority::normal, fs_root);
        fs_root.install_op<remote_op>(priority::normal, fs_root);
        fs_root.install_op<local_op>(priority::normal, fs_root);
        fs_root.install_op<session_dir_op>(priority::normal, fs_root);
        fs_root.install_op<client_op>(priority::normal);
        fs_root.install_op<server_op>(priority::normal, fs_root);
    }

    ~installer()
    {
        //try {
        //    fs_root_.uninstall_op<server_op>();
        //    fs_root_.uninstall_op<client_op>();
        //    fs_root_.uninstall_op<session_dir_op>();
        //    fs_root_.uninstall_op<local_op>();
        //    fs_root_.uninstall_op<remote_op>();
        //    fs_root_.uninstall_op<data_op>();
        //    fs_root_.uninstall_op<ctl_op>();
        //    fs_root_.uninstall_op<clone_op>();
        //    fs_root_.uninstall_op<dir_op>();
        //} catch (...) {
        //}
    }

    static bool is_matching_path(path_type const&, file_type)
    {
        return false;
    }
};


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_INSTALLER_HPP
