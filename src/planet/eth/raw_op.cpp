
#include <planet/common.hpp>
#include <planet/eth/raw_op.hpp>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>

namespace planet {
namespace net {
namespace eth {


    shared_ptr<planet_operation> raw_op::new_instance() const
    {
        return std::make_shared<raw_op>();
    }

    void raw_op::bind_to_interface(int fd, std::string const& ifname, int protocol)
    {
        sockaddr_ll sll = {};
        sll.sll_family      = AF_PACKET;
        sll.sll_halen       = IFHWADDRLEN;
        sll.sll_protocol    = protocol;
        sll.sll_ifindex     = if_nametoindex(ifname.c_str());
        if (!sll.sll_ifindex)
            throw planet::exception_errno(errno);
        if (::bind(fd, reinterpret_cast<sockaddr *>(&sll), sizeof (sll)) < 0)
            throw planet::exception_errno(errno);
    }

    int raw_op::do_raw_open(int sock_type, int protocol, std::string const& ifname)
    {
        int fd = socket(AF_PACKET, sock_type, protocol);
        if (fd < 0)
            throw planet::exception_errno(errno);
        bind_to_interface(fd, ifname, protocol);
        return fd;
    }

    int raw_op::open(shared_ptr<file_entry> file_ent, path_type const& path)
    {
        int socket_type, protocol = htons(ETH_P_ALL);
        if (path.parent_path() == "/eth")
            socket_type = SOCK_RAW;
        if (path.parent_path() == "/ip")
            socket_type = SOCK_DGRAM;
        fd_ = do_raw_open(socket_type, protocol, path.filename().string());
        ::syslog(LOG_NOTICE, "%s: opened fd=%d ifname=%s"
            , __PRETTY_FUNCTION__, fd_, path.filename().string().c_str());
        return 0;
    }

    int raw_op::read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        int bytes = ::recv(fd_, buf, size, 0);
        if (bytes < 0)
            throw exception_errno(errno);
        return bytes;
    }

    int raw_op::write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        int bytes = ::send(fd_, buf, size, 0);
        if (bytes < 0)
            throw exception_errno(errno);
        return bytes;
    }

    int raw_op::release(shared_ptr<file_entry> file_ent)
    {
        return close(fd_);
    }

    int raw_op::mknod(shared_ptr<file_entry>, path_type const&, mode_t, dev_t)
    {
        return 0;
    }

    int raw_op::rmnod(shared_ptr<file_entry>, path_type const&)
    {
        return -EPERM;
    }

    bool raw_op::is_matching_path(path_type const& path)
    {
        return (path.parent_path() == "/eth" || path.parent_path() == "/ip");
    }


}   // namespace eth
}   // namespace net
}   // namespace planet
