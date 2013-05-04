
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/tcp_accepted_client_op.hpp>
#include <sys/types.h>
#include <sys/socket.h>

namespace planet {


    shared_ptr<planet_operation> tcp_accepted_client_op::new_instance() const
    {
        return std::make_shared<tcp_accepted_client_op>();
    }

    int tcp_accepted_client_op::open(shared_ptr<file_entry> file_ent, path_type const& path)
    {
        // Get already opened file descriptor from vector
        fd_ = *reinterpret_cast<int *>(data_vector(*file_ent).data());
        ::syslog(LOG_NOTICE, "%s: found tcp socket fd=%d", __PRETTY_FUNCTION__, fd_);
        return 0;
    }

    int tcp_accepted_client_op::read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        return ::recv(fd_, buf, size, 0);
    }

    int tcp_accepted_client_op::write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        return ::send(fd_, buf, size, 0);
    }

    int tcp_accepted_client_op::release(shared_ptr<file_entry> file_ent)
    {
        return ::close(fd_);
    }

    bool tcp_accepted_client_op::is_matching_path(path_type const& path)
    {
        return false;
    }


}   // namespace planet
