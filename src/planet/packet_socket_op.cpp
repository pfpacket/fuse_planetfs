
#include <planet/common.hpp>
#include <planet/packet_socket_op.hpp>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <planet/packet_socket_op.hpp>

namespace planet {


    shared_ptr<planet_operation> packet_socket_op::new_instance() const
    {
        return std::make_shared<packet_socket_op>();
    }

    void packet_socket_op::bind_to_interface(int fd, std::string const& ifname, int protocol)
    {
        sockaddr_ll sll = {};
        sll.sll_family      = AF_PACKET;
        sll.sll_halen       = IFHWADDRLEN;
        sll.sll_protocol    = protocol;
        sll.sll_ifindex       = if_nametoindex(ifname.c_str());
        if (::bind(fd, reinterpret_cast<sockaddr *>(&sll), sizeof (sll)) < 0)
            throw planet::exception_errno(errno);
    }

    int packet_socket_op::do_packet_socket_open(int sock_type, int protocol, std::string const& ifname)
    {
        int fd = socket(AF_PACKET, sock_type, protocol);
        if (fd < 0)
            throw planet::exception_errno(errno);
        bind_to_interface(fd, ifname, protocol);
        return fd;
    }

    int packet_socket_op::open(shared_ptr<file_entry> file_ent, path_type const& path)
    {
        int socket_type, protocol = htons(ETH_P_ALL);
        if (path.parent_path() == "/eth")
            socket_type = SOCK_RAW;
        if (path.parent_path() == "/ip")
            socket_type = SOCK_DGRAM;
        fd_ = do_packet_socket_open(socket_type, protocol, path.filename().string());
        ::syslog(LOG_NOTICE, "%s: created packet socket fd=%d", __PRETTY_FUNCTION__, fd_);
        return 0;
    }

    int packet_socket_op::read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        return ::recv(fd_, buf, size, 0);
    }

    int packet_socket_op::write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        return ::send(fd_, buf, size, 0);
    }

    int packet_socket_op::release(shared_ptr<file_entry> file_ent)
    {
        return close(fd_);
    }

    bool packet_socket_op::is_matching_path(path_type const& path)
    {
        return (path.parent_path() == "/eth" || path.parent_path() == "/ip");
    }


}   // namespace planet
