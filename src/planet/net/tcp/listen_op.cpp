
#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/net/tcp/listen_op.hpp>

namespace planet {
namespace net {
namespace tcp {


    static int accept_new_connection(int server)
    {
        sockaddr_storage addr;
        auto addr_len = sizeof (addr);
        int new_socket = ::accept(server, (sockaddr *)&addr, &addr_len);
        if (new_socket < 0)
            throw_system_error(errno, "accept(sock=" + std::to_string(server) + ")");
        return new_socket;
    }

    static std::string open_new_session(shared_ptr<core_file_system> fs_root)
    {
        std::vector<char> buffer(64);
        auto clone = fs_root->open("/tcp/clone");
        raii_wrapper close_clone([clone, fs_root] { fs_root->close(clone); });
        int bytes_read = fs_root->read(clone, buffer.data(), buffer.size(), 0);
        // (bytes_read - 1): /tcp/clone returns new number, containing '\n'
        return std::string(buffer.data(), bytes_read - 1);
    }

    //
    // listen_op
    //
    listen_op::listen_op(shared_ptr<core_file_system> fs) : fs_root_(fs)
    {
    }

    int listen_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        path_ = path;
        return 0;
    }

    int listen_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        int server_socket = -1;
        if (auto sock = detail::fdtable.find_from_path(path_.string()))
            server_socket = *sock;
        else
            return -ENOTCONN;
        if (read_once_.load())
            return 0;

        int new_socket = accept_new_connection(server_socket);
        raii_wrapper raii([new_socket] {
            if (std::uncaught_exception())
                ::close(new_socket);
        });

        // Open clone to create new session
        auto&& session_str = open_new_session(fs_root_);
        if (size < session_str.length())
            return -ENAMETOOLONG;
        BOOST_LOG_TRIVIAL(debug) << "listen_op::read: server=" << server_socket
            << " new_socket=" << new_socket << " new_session=" << session_str;

        detail::fdtable.insert(session_str, new_socket);
        std::copy_n(session_str.data(), session_str.size(), buf);
        read_once_.store(true);
        return session_str.length();
    }

    int listen_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        return -EPERM;
    }

    int listen_op::release(shared_ptr<fs_entry> file_ent)
    {
        return 0;
    }

    //
    // listen_type
    //
    const string_type listen_type::type_name = "planet.net.tcp.listen";

    listen_type::listen_type() : file_ops_type(type_name)
    {
    }

    shared_ptr<entry_op> listen_type::create_op(shared_ptr<core_file_system> fs_root)
    {
        return make_shared<listen_op>(fs_root);
    }

    bool listen_type::match_path(path_type const& path, file_type type)
    {
        return type == file_type::regular_file
            && xpv::regex_match(path.string(), path_reg::listen);
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
