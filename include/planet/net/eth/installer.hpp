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


class installer : public entry_op {
private:
    shared_ptr<core_file_system> fs_root_;
public:
    installer(shared_ptr<core_file_system> fs_root) : fs_root_(fs_root)
    {
        typedef planet::core_file_system::priority priority;
        fs_root_->install_op<dir_op>(priority::normal);
        fs_root_->install_op<raw_op>(priority::normal);
    }

    ~installer()
    {
        //try {
        //    fs_root_->uninstall_op<raw_op>();
        //    fs_root_->uninstall_op<dir_op>();
        //} catch (...) {
        //}
    }

    static bool match_path(path_type const&, file_type)
    {
        return false;
    }
};


}   // namespace eth
}   // namespace net
}   // namespace planet

#endif  // PLANET_ETH_INSTALLER_HPP
