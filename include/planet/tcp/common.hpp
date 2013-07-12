#ifndef PLANET_TCP_COMMON_HPP
#define PLANET_TCP_COMMON_HPP

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

namespace planet {
namespace net {
namespace tcp {


    enum path_delimiter  : char  {
        host_port_delimiter = '!'
    };

    enum sock_arg : int {
        domain      = AF_INET,
        type        = SOCK_STREAM,
        protocol     = 0
    };


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_COMMON_HPP
