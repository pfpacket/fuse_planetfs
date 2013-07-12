#ifndef PLANET_TCP_COMMON_HPP
#define PLANET_TCP_COMMON_HPP

#include <planet/common.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <boost/regex.hpp>

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
        extern boost::regex ctl;
        extern boost::regex data;
        extern boost::regex local;
        extern boost::regex remote;
        extern boost::regex session_dir;
    }   // namespace path_reg


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_COMMON_HPP
