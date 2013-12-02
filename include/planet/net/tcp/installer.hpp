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


    class installer : public fs_ops_type {
    public:
        installer() : fs_ops_type("planet.net.tcp.installer")
        {
        }

        int install(shared_ptr<core_file_system> fs_root)
        {
            typedef planet::core_file_system::priority priority;
            fs_root->install_ops<dir_type>(priority::normal);
            fs_root->install_ops<clone_type>(priority::normal, 0);
            fs_root->install_ops<ctl_type>(priority::normal);
            fs_root->install_ops<data_type>(priority::normal);
            fs_root->install_ops<remote_type>(priority::normal);
            fs_root->install_ops<local_type>(priority::normal);
            fs_root->install_ops<session_dir_type>(priority::normal);
            fs_root->install_ops<client_type>(priority::normal);
            fs_root->install_ops<server_type>(priority::normal);
            return 0;
        }

        int uninstaller(shared_ptr<core_file_system> fs_root)
        {
            //try {
            //    if (fs_root_) {
            //        ::syslog(LOG_NOTICE, "tcp::installer: dtor: fs_root use_count=%ld", fs_root_.use_count());
            //        fs_root->uninstall_op<server_type>();
            //        fs_root->uninstall_op<client_type>();
            //        fs_root->uninstall_op<session_dir_type>();
            //        fs_root->uninstall_op<local_type>();
            //        fs_root->uninstall_op<remote_type>();
            //        fs_root->uninstall_op<data_type>();
            //        fs_root->uninstall_op<ctl_type>();
            //        fs_root->uninstall_op<clone_type>();
            //        fs_root->uninstall_op<dir_type>();
            //    }
            //} catch (...) {
            //}
            return 0;
        }

        bool match_path(path_type const&, file_type)
        {
            return false;
        }
    };


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_INSTALLER_HPP
