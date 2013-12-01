
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


    shared_ptr<entry_op> client_op::create_op()
    {
        return std::make_shared<client_op>(::planet::detail::shared_null_ptr);
    }

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

    int client_op::mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t, dev_t)
    {
        auto filename   = path.filename().string();
        auto pos        = filename.find_first_of(host_port_delimiter);
        auto host       = filename.substr(0, pos);
        auto port       = filename.substr(pos + 1);
        syslog(LOG_INFO, "client_op::mknod: connecting to host=%s, port=%s", host.c_str(), port.c_str());
        int sock = sock_connect_to(host, port);
        syslog(LOG_NOTICE, "client_op::mknod: connection established %s!%s fd=%d opened", host.c_str(), port.c_str(), sock);
        detail::fdtable.insert(path.string(), sock);
        return 0;
    }

    int client_op::rmnod(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        detail::fdtable.erase(path.string());
        return 0;
    }

    bool client_op::match_path(path_type const& path, file_type type)
    {
        return  type == file_type::regular_file &&
                path.parent_path() == "/tcp" &&
                path.filename().string()[0] != '*';
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
