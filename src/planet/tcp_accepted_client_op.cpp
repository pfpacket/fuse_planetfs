
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <planet/common.hpp>
#include <planet/tcp_accepted_client_op.hpp>

namespace planet {


int tcp_accepted_client_op::open(fusecpp::path_type const& path, struct fuse_file_info& fi)
{
    return 0;
}

inline int tcp_accepted_client_op::read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi)
{
    return ::recv(fd_, buf, size, 0);
}

inline int tcp_accepted_client_op::write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi)
{
    return ::send(fd_, buf, size, 0);
}

inline int tcp_accepted_client_op::release(fusecpp::path_type const& path, struct fuse_file_info& fi)
{
    return ::close(fd_);
}


}   // namespace planet
