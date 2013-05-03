
#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>
#include <planet/tcp_client_op.hpp>
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
        struct sockaddr_in sin = {AF_INET, htons(port), {0}, {0}};
        inet_pton(AF_INET, host.c_str(), &sin.sin_addr);
        if (connect(fd, reinterpret_cast<struct sockaddr *>(&sin), sizeof(sin)) < 0)
            throw planet::exception_errno(errno);
        return fd;
    }

    int tcp_client_op::open(shared_ptr<file_entry> file_ent, path_type const& path)
    {
        std::string filename = path.filename().string();
        auto pos = filename.find_first_of(host_port_delimiter);
        host_ = filename.substr(0, pos), port_ = atoi(filename.substr(pos + 1).c_str());
        syslog(LOG_INFO, "tcp_client_op::open: connecting to host=%s, port=%d fd=%d", host_.c_str(), port_, fd_);
        fd_ = connect_to(host_, port_);
        syslog(LOG_NOTICE, "tcp_client_op::open: connection established %s:%d fd=%d opened", host_.c_str(), port_, fd_);
        return fd_;
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
        return ::close(fd_);
    }

    bool tcp_client_op::is_matching_path(path_type const& path)
    {
        return (path.parent_path() == "/tcp"
                && path.filename().string()[0] != '*');
    }


}   // namespace planet
