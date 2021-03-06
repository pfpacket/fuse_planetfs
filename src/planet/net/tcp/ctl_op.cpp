
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
#include <planet/net/tcp/ctl_op.hpp>
#include <planet/utils.hpp>
#include <planet/request_parser.hpp>

namespace planet {
namespace net {
namespace tcp {


    const string_type ctl_type::type_name = "planet.net.tcp.ctl";

    int ctl_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        xpv::smatch m;
        xpv::regex_search(path.string(), m, path_reg::ctl);
        current_fd_ = lexical_cast<int>(m[1]);
        return 0;
    }

    int ctl_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        int ret = 0;
        if (!fd_number_already_read_) {
            std::string dir_number = str(format("%1%\n") % current_fd_);
            if (dir_number.length() > size)
                return -ENOBUFS;
            std::copy(dir_number.begin(), dir_number.end(), buf);
            ret = dir_number.length();
            fd_number_already_read_ = true;
        }
        return ret;
    }

    static xpv::sregex const reg_ipv4_addr_port =
        (xpv::s1=(xpv::repeat<3>(xpv::repeat<1,3>(xpv::_d) >> '.') >> xpv::repeat<1,3>(xpv::_d)))
            >> '!' >> (xpv::s2=xpv::repeat<1,6>(xpv::_d)) >> *xpv::as_xpr('\n');
    static xpv::sregex const reg_announce =
        (xpv::s1=xpv::as_xpr('*')) >> '!' >> (xpv::s2=xpv::repeat<1,6>(xpv::_d)) >> *xpv::as_xpr('\n');

    static int setup_tcp_server(std::string port)
    {
        int server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock < 0)
            throw_system_error(errno, "setup_tcp_server: server_sock(2)");
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof (addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(std::stoi(port));
        int ret = ::bind(server_sock, (sockaddr *)&addr, sizeof (addr));
        if (ret < 0)
            throw_system_error(errno, "setup_tcp_server: bind(2)");
        ret = ::listen(server_sock, 5);
        if (ret < 0)
            throw_system_error(errno, "setup_tcp_server: listen(2)");
        return server_sock;
    }

    int ctl_op::interpret_request(string_type const& request)
    {
        int ret = 0;
        request_parser parser;
        if (!parser.parse(request))
            return -ENOTSUP;

        auto socket = detail::fdtable.find(lexical_cast<string_type>(current_fd_));
        if (parser.get_command() == "is_connected") {
            if (!socket || !sock_is_connected(*socket))
                ret = -ENOTCONN;
        } else if (parser.get_command() == "is_announced") {
            if (!socket || !(sock_get_tcp_info(*socket).tcpi_state == TCP_LISTEN))
                ret = -ENOTCONN;
        } else if (parser.get_command() == "connect") {
            if (socket && sock_is_connected(*socket))
                return -EISCONN;
            auto&& args = parser.get_filtered_args(reg_ipv4_addr_port);
            if (args.empty())
                return -ENOTSUP;

            BOOST_LOG_TRIVIAL(info) << __func__ << ": connecting to " << args[0][1] << "!" << args[0][2];
            int new_sock = sock_connect_to(args[0][1], args[0][2]);
            detail::fdtable.insert(lexical_cast<string_type>(current_fd_), new_sock);
            BOOST_LOG_TRIVIAL(info) << __func__ << ": connected to " << args[0][1] << "!" << args[0][2];
        } else if (parser.get_command() == "announce") {
            auto&& args = parser.get_filtered_args(reg_announce);
            if (args.empty())
                return -ENOTSUP;
            BOOST_LOG_TRIVIAL(info) << __func__ << ": announcing " << args[0][1] << "!" << args[0][2];

            int server_sock = setup_tcp_server(args[0][2]);

            detail::fdtable.insert(lexical_cast<string_type>(current_fd_), server_sock);
        } else if (parser.get_command() == "hangup") {
            detail::fdtable.erase(lexical_cast<string_type>(current_fd_));
        } else if (parser.get_command() == "keepalive") {
            if (socket) {
                int optvalue = 1;
                if (::setsockopt(*socket, SOL_SOCKET, SO_KEEPALIVE, &optvalue, sizeof (optvalue)) < 0)
                    throw_system_error(errno);
                auto&& args = parser.get_filtered_args(R"((\d)+)");
                if (!args.empty()) {
                    optvalue = lexical_cast<int>(args[0][0]);
                    if (::setsockopt(*socket, IPPROTO_TCP, TCP_KEEPIDLE, &optvalue, sizeof (optvalue)) < 0)
                        throw_system_error(errno);
                }
            } else
                ret = -ENOTCONN;
        } else
            ret = -ENOTSUP;
        return ret;
    }

    int ctl_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        std::string request(buf, size);
        int result = interpret_request(request);
        return !result ? size : result;
    }

    int ctl_op::release(shared_ptr<fs_entry> file_ent)
    {
        return 0;
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
