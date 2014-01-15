#ifndef PLANET_ETH_INSTALLER_HPP
#define PLANET_ETH_INSTALLER_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/eth/dir_op.hpp>
#include <planet/net/eth/raw_op.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace eth {


    class installer : public fs_ops_type {
    private:
    public:
        installer() : fs_ops_type("planet.net.eth.installer")
        {
        }

        int install(shared_ptr<core_file_system> fs_root) override
        {
            typedef planet::core_file_system::priority priority;
            fs_root->install_ops<dir_type>(priority::normal);
            fs_root->install_ops<raw_type>(priority::normal);
            return 0;
        }

        int uninstall(shared_ptr<core_file_system> fs_root) override
        {
            fs_root->uninstall_ops(dir_type::type_name);
            fs_root->uninstall_ops(raw_type::type_name);
            return 0;
        }

        bool match_path(path_type const&, file_type) override
        {
            return false;
        }
    };


}   // namespace eth
}   // namespace net
}   // namespace planet

#endif  // PLANET_ETH_INSTALLER_HPP
