#ifndef PLANET_NET_TCP_LISTEN_OP_HPP
#define PLANET_NET_TCP_LISTEN_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/utils.hpp>

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

    class listen_op : public default_file_op {
    private:
        shared_ptr<core_file_system> fs_root_;
        path_type path_;
        std::atomic<bool> read_once_{false};
    public:
        listen_op(shared_ptr<core_file_system> fs) : fs_root_(fs)
        {
        }

        int open(shared_ptr<fs_entry> file_ent, path_type const& path) override
        {
            path_ = path;
            return 0;
        }

        int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
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

        int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
        {
            return -EPERM;
        }

        int release(shared_ptr<fs_entry> file_ent)
        {
            return 0;
        }
    };

    class listen_type : public file_ops_type {
    public:
        static const string_type type_name;
        listen_type() : file_ops_type(type_name)
        {
        }

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override
        {
            return make_shared<listen_op>(fs_root);
        }

        bool match_path(path_type const& path, file_type type) override
        {
            return type == file_type::regular_file
                && xpv::regex_match(path.string(), path_reg::listen);
        }
    };
    const string_type listen_type::type_name = "planet.net.tcp.listen";


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_NET_TCP_LISTEN_OP_HPP
