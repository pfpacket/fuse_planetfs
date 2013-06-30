
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
#include <planet/tcp/client_op.hpp>
#include <planet/utils.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>

namespace planet {
namespace net {
namespace tcp {


    shared_ptr<entry_operation> client_op::new_instance() const
    {
        return std::make_shared<client_op>();
    }

    int client_op::connect_to(std::string const& host, int port)
    {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            throw planet::exception_errno(errno);
        struct sockaddr_in sin = {AF_INET, htons(port), {0}, {0}};
        inet_pton(AF_INET, host.c_str(), &sin.sin_addr);
        if (connect(fd, reinterpret_cast<struct sockaddr *>(&sin), sizeof(sin)) < 0)
            throw planet::exception_errno(errno);
        return fd;
    }

    int client_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        fd_ = get_data_from_vector<int>(data_vector(*file_cast(file_ent)));
        return 0;
    }

    int client_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        int bytes = ::recv(fd_, buf, size, 0);
        if (bytes < 0)
            throw exception_errno(errno);
        return bytes;
    }

    int client_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        int bytes = ::send(fd_, buf, size, 0);
        if (bytes < 0)
            throw exception_errno(errno);
        return bytes;
    }

    int client_op::release(shared_ptr<fs_entry> file_ent)
    {
        return 0;
    }

    int client_op::mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t, dev_t)
    {
        auto filename = path.filename().string();
        auto pos = filename.find_first_of(host_port_delimiter);
        string_type host = filename.substr(0, pos);
        int port = atoi(filename.substr(pos + 1).c_str());
        syslog(LOG_INFO, "client_op::mknod: connecting to host=%s, port=%d", host.c_str(), port);
        int fd = connect_to(host, port);
        syslog(LOG_NOTICE, "client_op::mknod: connection established %s:%d fd=%d opened", host.c_str(), port, fd);
        store_data_to_vector(data_vector(*file_cast(file_ent)), fd);
        return 0;
    }

    int client_op::rmnod(shared_ptr<fs_entry> file_ent, path_type const&)
    {
        return ::close(
            get_data_from_vector<int>(data_vector(*file_cast(file_ent)))
        );
    }

    bool client_op::is_matching_path(path_type const& path, file_type type)
    {
        return  type == file_type::regular_file &&
                path.parent_path() == "/tcp" &&
                path.filename().string()[0] != '*';
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
