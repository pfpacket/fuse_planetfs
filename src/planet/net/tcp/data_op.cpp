
#include <planet/common.hpp>
#include <planet/net/tcp/data_op.hpp>
#include <planet/utils.hpp>
#include <syslog.h>

namespace planet {
namespace net {
namespace tcp {


    int data_op::open(shared_ptr<fs_entry> fs_ent, path_type const& path)
    {
        int ret = 0;
        if (auto sock = detail::fdtable.find_from_path(path.string()))
            socket_ = *sock;
        else
            ret = -ENOTCONN;
        return ret;
    }

    int data_op::read(shared_ptr<fs_entry> fs_ent, char *buf, size_t size, off_t offset)
    {
        int bytes = ::recv(socket_, buf, size, 0);
        if (bytes < 0)
            throw_system_error(errno);
        return bytes;
    }

    int data_op::write(shared_ptr<fs_entry> fs_ent, char const *buf, size_t size, off_t offset)
    {
        int bytes = ::send(socket_, buf, size, 0);
        if (bytes < 0)
            throw_system_error(errno);
        return bytes;
    }

    int data_op::release(shared_ptr<fs_entry> fs_ent)
    {
        return 0;
    }

    int data_op::poll(pollmask_t& pollmask)
    {
        // rfds, wfds, efds
        static std::vector<fd_set> fdsets(3);
        for (auto&& fdset : fdsets) {
            FD_ZERO(&fdset);
            FD_SET(socket_, &fdset);
        }
        struct timespec no_wait = {};
        int ret = pselect(socket_ + 1, &fdsets[0], &fdsets[1], &fdsets[2], &no_wait, nullptr);
        if (ret == -1)
            return -errno;
        for (unsigned i = 0; i < fdsets.size(); ++i) {
            if (FD_ISSET(socket_, &fdsets[i])) {
                if (i == 0)
                    pollmask |= POLLIN;
                else if (i == 1)
                    pollmask |= POLLOUT;
                else if (i == 2)
                    pollmask |= POLLERR;
            }
        }
        return 0;
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
