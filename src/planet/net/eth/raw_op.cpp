
#include <planet/common.hpp>
#include <planet/net/eth/raw_op.hpp>
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


    //
    // raw_op
    //
    void raw_op::bind_to_interface(int fd, std::string const& ifname, int protocol)
    {
        sockaddr_ll sll{};
        sll.sll_family      = AF_PACKET;
        sll.sll_halen       = IFHWADDRLEN;
        sll.sll_protocol    = protocol;
        sll.sll_ifindex     = if_nametoindex(ifname.c_str());
        if (!sll.sll_ifindex)
            throw_system_error(errno);
        if (::bind(fd, reinterpret_cast<sockaddr *>(&sll), sizeof (sll)) < 0)
            throw_system_error(errno);
    }

    int raw_op::do_raw_open(int sock_type, int protocol, std::string const& ifname)
    {
        int fd = socket(AF_PACKET, sock_type, protocol);
        if (fd < 0)
            throw_system_error(errno);
        bind_to_interface(fd, ifname, protocol);
        return fd;
    }

    int raw_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        int socket_type, protocol = htons(ETH_P_ALL);
        if (path.parent_path() == "/eth")
            socket_type = SOCK_RAW;
        if (path.parent_path() == "/ip")
            socket_type = SOCK_DGRAM;
        fd_ = do_raw_open(socket_type, protocol, path.filename().string());
        BOOST_LOG_TRIVIAL(info) << "raw_op::open: opened fd=" << fd_ << " ifname=" << path.filename();
        return 0;
    }

    int raw_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        int bytes = ::recv(fd_, buf, size, 0);
        if (bytes < 0)
            throw_system_error(errno);
        return bytes;
    }

    int raw_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        int bytes = ::send(fd_, buf, size, 0);
        if (bytes < 0)
            throw_system_error(errno);
        return bytes;
    }

    int raw_op::release(shared_ptr<fs_entry> file_ent)
    {
        return close(fd_);
    }

    int raw_op::poll(pollmask_t& pollmask)
    {
        // rfds, wfds, efds
        std::vector<fd_set> fdsets(3);
        for (auto&& fdset : fdsets) {
            FD_ZERO(&fdset);
            FD_SET(fd_, &fdset);
        }
        struct timespec no_wait = {};
        int ret = pselect(fd_ + 1, &fdsets[0], &fdsets[1], &fdsets[2], &no_wait, nullptr);
        if (ret == -1)
            return -errno;
        for (unsigned i = 0; i < fdsets.size(); ++i) {
            if (FD_ISSET(fd_, &fdsets[i])) {
                if (i == 0)
                    pollmask |= POLLIN;
                else if (i == 1)
                    pollmask |= POLLOUT;
                else if (i == 2)
                    pollmask |= POLLERR;
            }
        }
        return 0;
    }

    //
    // raw_type
    //
    const string_type raw_type::type_name = "planet.net.eth.raw";

    shared_ptr<entry_op> raw_type::create_op(shared_ptr<core_file_system> fs_root)
    {
        return make_shared<raw_op>(fs_root);
    }

    int raw_type::mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t)
    {
        return 0;
    }

    int raw_type::rmnod(shared_ptr<fs_entry>, path_type const&)
    {
        return -EPERM;
    }

    bool raw_type::match_path(path_type const& path, file_type type)
    {
        return type == file_type::regular_file &&
            (path.parent_path() == "/eth" || path.parent_path() == "/ip");
    }


}   // namespace eth
}   // namespace net
}   // namespace planet
