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


class installer : public fs_operation {
public:
    installer(core_file_system& fs_root)
    {
        typedef planet::core_file_system::priority priority;
        fs_root.install_op<dir_op>(priority::normal, fs_root);
        fs_root.install_op<raw_op>(priority::normal);
    }

    static bool is_matching_path(path_type const&, file_type)
    {
        return false;
    }
private:
};


}   // namespace eth
}   // namespace net
}   // namespace planet

#endif  // PLANET_ETH_INSTALLER_HPP
