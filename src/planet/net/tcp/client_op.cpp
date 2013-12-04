
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
#include <planet/net/tcp/client_op.hpp>
#include <planet/utils.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>

namespace planet {
namespace net {
namespace tcp {


    int client_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        if (auto fd = detail::fdtable.find(path.string()))
            fd_ = *fd;
        else
            throw std::runtime_error("client_op: open: cannot get socket from fdtable");
        return 0;
    }

    int client_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        int bytes = ::recv(fd_, buf, size, 0);
        if (bytes < 0)
            throw_system_error(errno);
        return bytes;
    }

    int client_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        int bytes = ::send(fd_, buf, size, 0);
        if (bytes < 0)
            throw_system_error(errno);
        return bytes;
    }

    int client_op::release(shared_ptr<fs_entry> file_ent)
    {
        return 0;
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
