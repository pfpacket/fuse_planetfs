
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <planet/common.hpp>
#include <planet/tcp_server_op.hpp>
#include <fusecpp.hpp>

extern int do_planet_mknod(fusecpp::path_type const&,
    mode_t, dev_t, planet::opcode, std::function<void (fusecpp::file&)>);

namespace planet {


tcp_server_op::~tcp_server_op()
{
    syslog(LOG_INFO, "tcp_server_op: dtor called fd=%d", fd_);
}

static int do_tcp_server_open(std::string const& host, int port)
{
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sin = {AF_INET, htons(port), {INADDR_ANY}, {0}};
    if (::bind(fd, reinterpret_cast<sockaddr *>(&sin), sizeof (sin)) < 0)
        throw planet::exception_errno(errno);
    if (::listen(fd, 5) < 0)
        throw planet::exception_errno(errno);
    return fd;
}

int tcp_server_op::open(fusecpp::path_type const& path, struct fuse_file_info& fi)
{
    std::string filename = path.filename().string();
    auto pos = filename.find_first_of(':');
    host_ = filename.substr(0, pos), port_ = atoi(filename.substr(pos + 1).c_str());
    syslog(LOG_INFO, "tcp_server_op::open: establishing server host=%s, port=%d", host_.c_str(), port_);

    fd_ = do_tcp_server_open(host_, port_);

    syslog(LOG_NOTICE, "tcp_server_op::open: established server %s:%d fd=%d opened", host_.c_str(), port_, fd_);
    return fd_;
}

int tcp_server_op::read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi)
{
    sockaddr_in client;
    socklen_t len = sizeof (client);
    int client_fd = accept(fd_, reinterpret_cast<sockaddr *>(&client), &len);
    if (client_fd < 0)
        throw planet::exception_errno(errno);
    std::string new_client_path = "/eth/ip/tcp/";
    new_client_path += inet_ntoa(client.sin_addr);
    new_client_path += (":" + std::to_string(ntohs(client.sin_port)));
    if (size <= new_client_path.length())
        throw planet::exception_errno(EOVERFLOW);
    do_planet_mknod(new_client_path.c_str(), S_IFREG | S_IRWXU, 0, opcode::tcp_accepted_client,
        [client_fd](fusecpp::file& f) {
            f.data().reserve(sizeof (int));
            *reinterpret_cast<int *>(fusecpp::get_file_data(f)) = client_fd;
        }
    );
    // "./net" is a dirty quick hack (what should I do ?)
    std::string userspace_path = "./net" + new_client_path;
    std::copy(userspace_path.begin(), userspace_path.end(), buf);
    buf[userspace_path.length()] = '\0';
    return userspace_path.length() + 1;
}

inline int tcp_server_op::write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi)
{
    return ::send(fd_, buf, size, 0);
}

inline int tcp_server_op::release(fusecpp::path_type const& path, struct fuse_file_info& fi)
{
    return ::close(fd_);
}

bool tcp_server_op::is_matching_path(fusecpp::path_type const& path)
{
    return (path.parent_path() == "/eth/ip/tcp"
            && path.filename().string()[0] == '*');
}


}   // namespace planet
