#ifndef PLANET_TCP_INSTALLER_HPP
#define PLANET_TCP_INSTALLER_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/net/tcp/dir_op.hpp>
#include <planet/net/tcp/clone_op.hpp>
#include <planet/net/tcp/ctl_op.hpp>
#include <planet/net/tcp/data_op.hpp>
#include <planet/net/tcp/listen_op.hpp>
#include <planet/net/tcp/address_op.hpp>
#include <planet/net/tcp/session_dir_op.hpp>
#include <planet/net/tcp/client_op.hpp>
#include <planet/net/tcp/server_op.hpp>
#include <planet/fs_core.hpp>
#include <planet/utils.hpp>

namespace planet {
namespace net {
namespace tcp {


    class installer : public fs_ops_type {
    public:
        static const string_type type_name;
        installer() : fs_ops_type(type_name)
        {
        }

        int install(shared_ptr<core_file_system> fs_root) override
        {
            typedef planet::core_file_system::priority priority;
            fs_root->install_ops<dir_type>(priority::normal);
            fs_root->install_ops<clone_type>(priority::normal, 0);
            fs_root->install_ops<ctl_type>(priority::normal);
            fs_root->install_ops<data_type>(priority::normal);
            fs_root->install_ops<remote_type>(priority::normal);
            fs_root->install_ops<local_type>(priority::normal);
            fs_root->install_ops<listen_type>(priority::normal);
            fs_root->install_ops<session_dir_type>(priority::normal);
            fs_root->install_ops<client_type>(priority::normal);
            fs_root->install_ops<server_type>(priority::normal);
            return 0;
        }

        int uninstall(shared_ptr<core_file_system> fs_root) override
        {
            try {
                BOOST_LOG_TRIVIAL(trace) << "tcp.installer: recursive uninstallation for TCP dependencies";
                fs_root->uninstall_ops(server_type::type_name);
                fs_root->uninstall_ops(client_type::type_name);
                fs_root->uninstall_ops(session_dir_type::type_name);
                fs_root->uninstall_ops(listen_type::type_name);
                fs_root->uninstall_ops(local_type::type_name);
                fs_root->uninstall_ops(remote_type::type_name);
                fs_root->uninstall_ops(data_type::type_name);
                fs_root->uninstall_ops(ctl_type::type_name);
                fs_root->uninstall_ops(clone_type::type_name);
                fs_root->uninstall_ops(dir_type::type_name);
            } catch (...) {
            }
            return 0;
        }

        bool match_path(path_type const&, file_type)
        {
            return false;
        }
    };
    const string_type installer::type_name = "planet.net.tcp.installer";


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_INSTALLER_HPP
