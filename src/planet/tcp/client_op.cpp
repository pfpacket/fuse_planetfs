
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
#include <planet/tcp/client_op.hpp>
#include <planet/utils.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>

namespace planet {


    shared_ptr<planet_operation> tcp_client_op::new_instance() const
    {
        return std::make_shared<tcp_client_op>();
    }

    int tcp_client_op::connect_to(std::string const& host, int port)
    {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            throw planet::exception_errno(errno);
        struct sockaddr_in sin = {AF_INET, htons(port), {0}, {0}};
        inet_pton(AF_INET, host.c_str(), &sin.sin_addr);
        if (connect(fd, reinterpret_cast<struct sockaddr *>(&sin), sizeof(sin)) < 0)
            throw planet::exception_errno(errno);
        return fd;
    }

    int tcp_client_op::open(shared_ptr<file_entry> file_ent, path_type const& path)
    {
        fd_ = get_data_from_vector<int>(data_vector(*file_ent));
        return 0;
    }

    int tcp_client_op::read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        return ::recv(fd_, buf, size, 0);
    }

    int tcp_client_op::write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        return ::send(fd_, buf, size, 0);
    }

    int tcp_client_op::release(shared_ptr<file_entry> file_ent)
    {
        return 0;
    }

    int tcp_client_op::mknod(shared_ptr<file_entry> file_ent, path_type const& path, mode_t, dev_t)
    {
        auto filename = path.filename().string();
        auto pos = filename.find_first_of(host_port_delimiter);
        string_type host = filename.substr(0, pos);
        int port = atoi(filename.substr(pos + 1).c_str());
        syslog(LOG_INFO, "tcp_client_op::mknod: connecting to host=%s, port=%d", host.c_str(), port);
        int fd = connect_to(host, port);
        syslog(LOG_NOTICE, "tcp_client_op::mknod: connection established %s:%d fd=%d opened", host.c_str(), port, fd);
        store_data_to_vector(data_vector(*file_ent), fd);
        return 0;
    }

    int tcp_client_op::rmnod(shared_ptr<file_entry> file_ent, path_type const&)
    {
        return ::close(
            get_data_from_vector<int>(data_vector(*file_ent))
        );
    }

    bool tcp_client_op::is_matching_path(path_type const& path)
    {
        return (path.parent_path() == "/tcp"
                && path.filename().string()[0] != '*');
    }


}   // namespace planet
