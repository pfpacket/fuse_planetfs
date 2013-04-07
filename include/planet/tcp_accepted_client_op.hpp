#ifndef PLANET_TCP_ACCEPTED_CLIENT_OP_HPP
#define PLANET_TCP_ACCEPTED_CLIENT_OP_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <planet/planet_handle.hpp>
#include <fusecpp.hpp>

namespace planet {

class tcp_accepted_client_op : public planet_operation {
private:
    int fd_;
public:
    tcp_accepted_client_op(fusecpp::path_type const& path, fusecpp::directory& root)
    {
        auto ptr = fusecpp::search_file(root, path);
        fd_ = *reinterpret_cast<int *>(fusecpp::get_file_data(*ptr));
    }
    virtual ~tcp_accepted_client_op()
    {
        syslog(LOG_INFO, "tcp_accepted_client_op: dtor called fd=%d", fd_);
    }
    int open(fusecpp::path_type const& path, struct fuse_file_info& fi);
    int read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi);
    int write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi);
    int release(fusecpp::path_type const& path, struct fuse_file_info& fi);

    static bool is_matching_path(fusecpp::path_type const&);
};

}   // namespace planet

#endif  // PLANET_TCP_ACCEPTED_CLIENT_OP_HPP
