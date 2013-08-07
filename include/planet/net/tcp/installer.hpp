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
    shared_ptr<core_file_system> fs_root_;
public:
    installer(shared_ptr<core_file_system> fs_root) : fs_root_(fs_root)
    {
        typedef planet::core_file_system::priority priority;
        fs_root_->install_op<dir_op>(priority::normal);
        fs_root_->install_op<clone_op>(priority::normal, 0);
        fs_root_->install_op<ctl_op>(priority::normal);
        fs_root_->install_op<data_op>(priority::normal);
        fs_root_->install_op<remote_op>(priority::normal);
        fs_root_->install_op<local_op>(priority::normal);
        fs_root_->install_op<session_dir_op>(priority::normal);
        fs_root_->install_op<client_op>(priority::normal);
        fs_root_->install_op<server_op>(priority::normal);
    }

    ~installer()
    {
        //try {
        //    if (fs_root_) {
        //        ::syslog(LOG_NOTICE, "tcp::installer: dtor: fs_root use_count=%ld", fs_root_.use_count());
        //        fs_root_->uninstall_op<server_op>();
        //        fs_root_->uninstall_op<client_op>();
        //        fs_root_->uninstall_op<session_dir_op>();
        //        fs_root_->uninstall_op<local_op>();
        //        fs_root_->uninstall_op<remote_op>();
        //        fs_root_->uninstall_op<data_op>();
        //        fs_root_->uninstall_op<ctl_op>();
        //        fs_root_->uninstall_op<clone_op>();
        //        fs_root_->uninstall_op<dir_op>();
        //    }
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
