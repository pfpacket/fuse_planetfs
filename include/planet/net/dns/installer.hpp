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
        static const string_type type_name;
        installer() : file_ops_type(type_name)
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
            fs_root->uninstall_ops(resolver_type::type_name);
            return 0;
        }
    
        bool match_path(path_type const&, file_type) override
        {
            return false;
        }
    };
    const string_type installer::type_name = "planet.net.dns.installer";


}   // namespace dns
}   // namespace net
}   // namespace planet

#endif  // PLANET_DNS_INSTALLER_HPP
