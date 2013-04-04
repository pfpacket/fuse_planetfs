#ifndef PLANET_TCP_SERVER_OP_HPP
#define PLANET_TCP_SERVER_OP_HPP

#include <sys/types.h>
#include <string>
#include <fuse/fuse.h>
#include <planet/planet_handle.hpp>

namespace planet {

class tcp_server_op : public planet_operation {
    int port_, fd_;
    std::string host_;

public:
    virtual ~tcp_server_op();
    int open(fusecpp::path_type const& path, struct fuse_file_info& fi);
    int read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi);
    int write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi);
    int release(fusecpp::path_type const& path, struct fuse_file_info& fi);
};

}   // namespace planet

#endif  // PLANET_TCP_SERVER_OP_HPP

