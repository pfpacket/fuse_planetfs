
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/utils.hpp>
#include <planet/tcp_server_op.hpp>
#include <planet/tcp_accepted_client_op.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>

namespace planet {


    shared_ptr<planet_operation> tcp_server_op::new_instance() const
    {
        return std::make_shared<tcp_server_op, core_file_system&>(fs_root_);
    }

    int tcp_server_op::do_tcp_server_open(std::string const& host, int port)
    {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in sin = {AF_INET, htons(port), {INADDR_ANY}, {0}};
        if (::bind(fd, reinterpret_cast<sockaddr *>(&sin), sizeof (sin)) < 0)
            throw planet::exception_errno(errno);
        if (::listen(fd, 5) < 0)
            throw planet::exception_errno(errno);
        return fd;
    }

    int tcp_server_op::open(shared_ptr<file_entry> file_ent, path_type const& path)
    {
        std::string filename = path.filename().string();
        auto pos = filename.find_first_of(host_port_delimiter);
        host_ = filename.substr(0, pos), port_ = atoi(filename.substr(pos + 1).c_str());
        syslog(LOG_INFO, "tcp_server_op::open: establishing server host=%s, port=%d", host_.c_str(), port_);

        fd_ = do_tcp_server_open(host_, port_);

        syslog(LOG_NOTICE, "tcp_server_op::open: established server %s:%d fd=%d opened", host_.c_str(), port_, fd_);
        return fd_;
    }

    int tcp_server_op::read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        sockaddr_in client;
        socklen_t len = sizeof (client);
        int client_fd = accept(fd_, reinterpret_cast<sockaddr *>(&client), &len);
        if (client_fd < 0)
            throw planet::exception_errno(errno);
        std::string new_client_path = "/tcp/";
        new_client_path += inet_ntoa(client.sin_addr);
        new_client_path += (host_port_delimiter + std::to_string(ntohs(client.sin_port)));
        if (size <= new_client_path.length())
            throw planet::exception_errno(EOVERFLOW);

        fs_root_.install_op<planet::tcp_accepted_client_op>();
        fs_root_.mknod(new_client_path, S_IFREG | S_IRWXU, 0, typeid(tcp_accepted_client_op));
        auto client_entry = file_cast(fs_root_.get_entry_of(new_client_path));
        auto& data = data_vector(*client_entry);
        data.reserve(sizeof (int));
        *reinterpret_cast<int *>(data.data()) = client_fd;

        // "./net" is a dirty quick hack (what should I do ?)
        std::string userspace_path = "./net" + new_client_path;
        std::copy(userspace_path.begin(), userspace_path.end(), buf);
        buf[userspace_path.length()] = '\0';
        return userspace_path.length() + 1;
    }

    int tcp_server_op::write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        return ::send(fd_, buf, size, 0);
    }

    int tcp_server_op::release(shared_ptr<file_entry> file_ent)
    {
        return ::close(fd_);
    }

    bool tcp_server_op::is_matching_path(path_type const& path)
    {
        return (path.parent_path() == "/tcp"
                && path.filename().string()[0] == '*');
    }


}   // namespace planet
