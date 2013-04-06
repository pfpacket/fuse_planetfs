
#include <string>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <planet/packet_socket_op.hpp>

namespace planet {

static void bind_to_interface(int fd, std::string const& ifname, int protocol)
{
    sockaddr_ll sll = {};
    sll.sll_family      = AF_PACKET;
    sll.sll_halen       = IFHWADDRLEN;
    sll.sll_protocol    = protocol;
    sll.sll_ifindex       = if_nametoindex(ifname.c_str());
    if (::bind(fd, reinterpret_cast<sockaddr *>(&sll), sizeof (sll)) < 0)
        throw planet::exception_errno(errno);
}

static int do_packet_socket_open(int sock_type, int protocol, std::string const& ifname)
{
    int fd = socket(AF_PACKET, sock_type, protocol);
    if (fd < 0)
        throw planet::exception_errno(errno);
    bind_to_interface(fd, ifname, protocol);
    return fd;
}

int packet_socket_op::open(fusecpp::path_type const& path, struct fuse_file_info& fi)
{
    int socket_type, protocol = htons(ETH_P_ALL);
    if (path.parent_path() == "/eth")
        socket_type = SOCK_RAW;
    if (path.parent_path() == "/eth/ip")
        socket_type = SOCK_DGRAM;
    fd_ = do_packet_socket_open(socket_type, protocol, path.filename().string());
    return 0;
}

int packet_socket_op::read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi)
{
    return ::recv(fd_, buf, size, 0);
}

int packet_socket_op::write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi)
{
    return ::send(fd_, buf, size, 0);
}

int packet_socket_op::release(fusecpp::path_type const& path, struct fuse_file_info& fi)
{
    return close(fd_);
}


}   // namespace planet
