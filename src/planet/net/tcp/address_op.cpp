
#include <planet/common.hpp>
#include <planet/net/tcp/address_op.hpp>
#include <planet/utils.hpp>
#include <sys/socket.h>

namespace planet {
namespace net {
namespace tcp {


    //
    // local_op - /tcp/*/local
    //
    const string_type local_type::type_name = "planet.net.tcp.local";

    int local_op::update_address(int sock, shared_ptr<fs_entry> file)
    {
        struct sockaddr_storage peer{};
        socklen_t len = sizeof (peer);
        if (getsockname(sock, (sockaddr *)&peer, &len) < 0)
            throw_system_error(errno, str(format("getsockname: fd=%1%") % sock));
        std::string hostname, servname;
        get_name_info((sockaddr *)&peer, len, hostname, servname);
        auto local_addr = str(format("%1%!%2%\n") % hostname % servname);
        file_cast(file)->data().clear();
        return default_file_op::write(file, local_addr.data(), local_addr.length(), 0);
    }

    int local_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        int ret = 0;
        if (auto sock = detail::fdtable.find_from_path(path.string()))
            sock_ = *sock;
        else
            ret = -ENOTCONN;
        return ret;
    }

    int local_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        this->update_address(*sock_, file_ent);
        return default_file_op::read(file_ent, buf, size, offset);
    }

    int local_op::write(shared_ptr<fs_entry>, char const *buf, size_t size, off_t offset)
    {
        return -EPERM;
    }

    //
    // remote_op - /tcp/*/remote
    //
    const string_type remote_type::type_name = "planet.net.tcp.remote";

    int remote_op::update_address(int sock, shared_ptr<fs_entry> file)
    {
        struct sockaddr_storage peer{};
        socklen_t len = sizeof (peer);
        if (getpeername(sock, (sockaddr *)&peer, &len) < 0)
            throw_system_error(errno, str(format("getpeername: fd=%1%") % sock));
        std::string hostname, servname;
        get_name_info((sockaddr *)&peer, len, hostname, servname);
        auto local_addr = str(format("%1%!%2%\n") % hostname % servname);
        file_cast(file)->data().clear();
        return default_file_op::write(file, local_addr.data(), local_addr.length(), 0);
    }

    int remote_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        int ret = 0;
        if (auto sock = detail::fdtable.find_from_path(path.string()))
            sock_ = sock;
        else
            ret = -ENOTCONN;
        return ret;
    }

    int remote_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        this->update_address(*sock_, file_ent);
        return default_file_op::read(file_ent, buf, size, offset);
    }

    int remote_op::write(shared_ptr<fs_entry>, char const *buf, size_t size, off_t offset)
    {
        return -EPERM;
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
