#ifndef PLANET_PACKET_SOCKET_OP_HPP
#define PLANET_PACKET_SOCKET_OP_HPP

#include <boost/filesystem/path.hpp>
#include <planet/common.hpp>
#include <planet/planet_handle.hpp>

namespace planet {

class packet_socket_op : public planet_operation {
    int fd_;

public:
    virtual ~packet_socket_op() = default;
    int open(fusecpp::path_type const& path, struct fuse_file_info& fi);
    int read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi);
    int write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi);
    int release(fusecpp::path_type const& path, struct fuse_file_info& fi);

    static bool is_matching_path(fusecpp::path_type const&);
};

}   // namespace planet

#endif  // PLANET_PACKET_SOCKET_OP_HPP
