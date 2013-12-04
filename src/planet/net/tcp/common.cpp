
#include <planet/net/tcp/common.hpp>
#include <planet/utils.hpp>

namespace planet {
namespace net {
namespace tcp {


    namespace path_reg {
        xpv::sregex ctl         = xpv::sregex::compile(R"(/tcp/(\d+)/ctl)");
        xpv::sregex data        = xpv::sregex::compile(R"(/tcp/(\d+)/data)");
        xpv::sregex local       = xpv::sregex::compile(R"(/tcp/(\d+)/local)");
        xpv::sregex remote      = xpv::sregex::compile(R"(/tcp/(\d+)/remote)");
        xpv::sregex session_dir = xpv::sregex::compile(R"(/tcp/(\d+))");
    }   // namespace path_reg

    int sock_connect_to(string_type const& host, string_type const& port)
    {
        int s, sock = -1;
        struct addrinfo hints, *res;
        std::memset(&hints, 0, sizeof (hints));
        hints.ai_family     = AF_UNSPEC;
        hints.ai_socktype   = SOCK_STREAM;
        hints.ai_flags      = AI_PASSIVE;

        s = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
        if (s != 0)
            throw std::runtime_error(gai_strerror(s));
        auto result = make_unique_ptr(res, [](struct addrinfo *ptr){ freeaddrinfo(ptr); });

        for (struct addrinfo *ai = result.get(); ai; ai = ai->ai_next) {
            sock = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            if (sock < 0)
                throw_system_error(errno);
            if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0 && ai->ai_next == nullptr)
                throw_system_error(errno);
        }
        if (sock < 0)
            throw_system_error(EHOSTUNREACH);
        return sock;
    }

    struct tcp_info sock_get_tcp_info(int sock)
    {
        struct tcp_info info;
        socklen_t info_length = sizeof (info);
        if (getsockopt(sock, SOL_TCP, TCP_INFO, &info, &info_length) != 0)
            throw_system_error(errno);
        return info;
    }

    // getsockopt(TCP_INFO) depends on Linux functionality
    bool sock_is_connected(int sock)
    {
        return sock_get_tcp_info(sock).tcpi_state == TCP_ESTABLISHED;
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
