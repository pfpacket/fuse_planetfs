
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


}   // namespace tcp
}   // namespace net
}   // namespace planet
