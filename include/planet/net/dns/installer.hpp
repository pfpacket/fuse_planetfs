#ifndef PLANET_DNS_INSTALLER_HPP
#define PLANET_DNS_INSTALLER_HPP

#include <planet/common.hpp>
#include <planet/net/dns/resolver_op.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace dns {


class installer : public fs_operation {
public:
    installer(core_file_system& fs_root)
    {
        typedef planet::core_file_system::priority priority;
        fs_root.install_op<resolver_op>(priority::normal);
    }

    static bool is_matching_path(path_type const&, file_type)
    {
        return false;
    }
private:
};


}   // namespace dns
}   // namespace net
}   // namespace planet

#endif  // PLANET_DNS_INSTALLER_HPP
