
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/utils.hpp>
#include <planet/tcp/server_op.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>

namespace planet {
namespace net {
namespace tcp {


    shared_ptr<fs_operation> server_op::new_instance() const
    {
        return std::make_shared<server_op, core_file_system&>(fs_root_);
    }

    int server_op::establish_server(std::string const& host, int port)
    {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in sin = {AF_INET, htons(port), {INADDR_ANY}, {0}};
        if (::bind(fd, reinterpret_cast<sockaddr *>(&sin), sizeof (sin)) < 0)
            throw planet::exception_errno(errno);
        if (::listen(fd, 5) < 0)
            throw planet::exception_errno(errno);
        return fd;
    }

    int server_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        server_fd_ = get_data_from_vector<int>(data_vector(*file_cast(file_ent)));
        sockaddr_in client;
        socklen_t len = sizeof (client);
        client_fd_ = accept(server_fd_, reinterpret_cast<sockaddr *>(&client), &len);
        if (client_fd_ < 0)
            throw planet::exception_errno(errno);
        return 0;
    }

    int server_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        int bytes = ::recv(client_fd_, buf, size, 0);
        if (bytes < 0)
            throw exception_errno(errno);
        return bytes;
    }

    int server_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        int bytes = ::send(client_fd_, buf, size, 0);
        if (bytes < 0)
            throw exception_errno(errno);
        return bytes;
    }

    int server_op::release(shared_ptr<fs_entry> file_ent)
    {
        return ::close(client_fd_);
    }

    int server_op::mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t, dev_t)
    {
        std::string filename = path.filename().string();
        auto pos = filename.find_first_of(host_port_delimiter);
        string_type host = filename.substr(0, pos);
        int port = atoi(filename.substr(pos + 1).c_str());
        syslog(LOG_INFO, "server_op::mknod: establishing server host=%s, port=%d", host.c_str(), port);
        int serverfd = establish_server(host, port);
        syslog(LOG_NOTICE, "server_op::mknod: established server %s:%d fd=%d opened", host.c_str(), port, serverfd);
        store_data_to_vector(data_vector(*file_cast(file_ent)), serverfd);
        return 0;
    }

    int server_op::rmnod(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        return ::close(
            get_data_from_vector<int>(data_vector(*file_cast(file_ent)))
        );
    }

    bool server_op::is_matching_path(path_type const& path, file_type type)
    {
        return  type == file_type::regular_file &&
                path.parent_path() == "/tcp" &&
                path.filename().string()[0] == '*';
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
