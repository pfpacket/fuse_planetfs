#ifndef PLANET_TCP_COMMON_HPP
#define PLANET_TCP_COMMON_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <netinet/tcp.h>

namespace planet {
namespace net {
namespace tcp {


    enum path_delimiter  : char  {
        host_port_delimiter = '!'
    };

    enum sock_arg : int {
        domain      = AF_INET,
        type        = SOCK_STREAM,
        protocol    = 0,

        domain6     = AF_INET6,
        type6       = SOCK_STREAM,
        protocol6   = 0
    };

    // Regex for files
    namespace path_reg {
        extern xpv::sregex ctl;
        extern xpv::sregex data;
        extern xpv::sregex local;
        extern xpv::sregex remote;
        extern xpv::sregex listen;
        extern xpv::sregex session_dir;
    }   // namespace path_reg

    extern int sock_connect_to(string_type const& host, string_type const& port);
    extern struct tcp_info sock_get_tcp_info(int);
    extern bool sock_is_connected(int);


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_COMMON_HPP
