
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <planet/common.hpp>
#include <planet/planet_tcp_client_op.hpp>

namespace planet {


inline tcp_client_op::~tcp_client_op()
{
    syslog(LOG_INFO, "tcp_client_op: dtor called fd=%d", fd_);
}

int tcp_client_op::open(fusecpp::path_type const& path, struct fuse_file_info& fi)
{
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    std::string filename = path.filename().string();
    auto pos = filename.find_first_of(':');
    host_ = filename.substr(0, pos), port_ = atoi(filename.substr(pos + 1).c_str());
    syslog(LOG_INFO, "tcp_client_op::open: connecting to host=%s, port=%d fd=%d", host_.c_str(), port_, fd_);

    struct sockaddr_in sin = {AF_INET, htons(port_), {0}, {0}};
    inet_pton(AF_INET, host_.c_str(), &sin.sin_addr);
    if (connect(fd_, reinterpret_cast<struct sockaddr *>(&sin), sizeof(sin)) < 0)
        throw -errno;
    syslog(LOG_NOTICE, "tcp_client_op::open: connection established %s:%d fd=%d opened", host_.c_str(), port_, fd_);
    return fd_;
}

inline int tcp_client_op::read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi)
{
    return ::recv(fd_, buf, size, 0);
}

inline int tcp_client_op::write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi)
{
    return ::send(fd_, buf, size, 0);
}

inline int tcp_client_op::release(fusecpp::path_type const& path, struct fuse_file_info& fi)
{
    return ::close(fd_);
}


}   // namespace planet
