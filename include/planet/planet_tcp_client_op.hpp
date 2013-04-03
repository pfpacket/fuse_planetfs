#ifndef PLANET_TCP_CLIENT_OP_HPP
#define PLANET_TCP_CLIENT_OP_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <planet/planet_handle.hpp>


class planet_tcp_client_ops : public planet_operations {
    int port_, fd_;
    std::string host_;
    std::vector<std::string> resolved_names_;
public:
    virtual ~planet_tcp_client_ops();
    int open(fusecpp::path_type const& path, struct fuse_file_info& fi);
    int read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi);
    int write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi);
    int release(fusecpp::path_type const& path, struct fuse_file_info& fi);
};


#endif  // PLANET_TCP_CLIENT_OP_HPP
