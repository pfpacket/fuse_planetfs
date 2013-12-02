#ifndef PLANET_DNS_INSTALLER_HPP
#define PLANET_DNS_INSTALLER_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/dns/resolver_op.hpp>
#include <planet/fs_ops_type.hpp>

namespace planet {
namespace net {
namespace dns {


    class installer final : public file_ops_type {
    public:
        installer() : file_ops_type("planet.net.dns.installer")
        {
        }
    
        virtual int install(shared_ptr<core_file_system> fs_root)
        {
            typedef planet::core_file_system::priority priority;
            fs_root->install_ops<resolver_type>(priority::normal);
            return 0;
        }

        int uninstall(shared_ptr<core_file_system> fs_root)
        {
            //fs_root->uninstall_op("net_dns_resolver");
            return 0;
        }
    
        bool match_path(path_type const&, file_type) override
        {
            return false;
        }
    };


}   // namespace dns
}   // namespace net
}   // namespace planet

#endif  // PLANET_DNS_INSTALLER_HPP
