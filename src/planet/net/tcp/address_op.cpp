
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
    void local_op::update_address(int sock, shared_ptr<fs_entry> file)
    {
        struct sockaddr_storage peer{};
        socklen_t len = sizeof (peer);
        if (getsockname(sock, (sockaddr *)&peer, &len) < 0)
            throw_system_error(errno, str(format("getpeername: fd=%1%") % sock));
        std::string hostname, servname;
        get_name_info((sockaddr *)&peer, len, hostname, servname);
        auto local_addr = str(format("%1%!%2%\n") % hostname % servname);
        file_cast(file)->data().clear();
        this->write(file, local_addr.c_str(), local_addr.length(), 0);
    }

    int local_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        int ret = 0;
        ::syslog(LOG_INFO, "%s: %s", __PRETTY_FUNCTION__, path.parent_path().string().c_str());
        if (auto sock = detail::fdtable.find_from_path(path.string()))
            update_address(*sock, file_ent);
        else
            ret = -ENOTCONN;
        return ret;
    }

    //
    // remote_op - /tcp/*/remote
    //
    void remote_op::update_address(int sock, shared_ptr<fs_entry> file)
    {
        struct sockaddr_storage peer{};
        socklen_t len = sizeof (peer);
        if (getpeername(sock, (sockaddr *)&peer, &len) < 0)
            throw_system_error(errno, str(format("getpeername: fd=%1%") % sock));
        std::string hostname, servname;
        get_name_info((sockaddr *)&peer, len, hostname, servname);
        auto local_addr = str(format("%1%!%2%\n") % hostname % servname);
        file_cast(file)->data().clear();
        this->write(file, local_addr.c_str(), local_addr.length(), 0);
    }

    int remote_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        int ret = 0;
        ::syslog(LOG_INFO, "%s: %s", __PRETTY_FUNCTION__, path.parent_path().string().c_str());
        if (auto sock = detail::fdtable.find_from_path(path.string()))
            update_address(*sock, file_ent);
        else
            ret = -ENOTCONN;
        return ret;
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
