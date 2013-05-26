#ifndef PLANET_PACKET_SOCKET_HPP
#define PLANET_PACKET_SOCKET_HPP

#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {


class packet_socket_op : public planet_operation {
private:
    int fd_;

    static void bind_to_interface(int fd, std::string const& ifname, int protocol);
    static int do_packet_socket_open(int sock_type, int protocol, std::string const& ifname);

public:
    packet_socket_op()
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }
    ~packet_socket_op() noexcept
    {
        ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
    }

    shared_ptr<planet_operation> new_instance() const;
    int open(shared_ptr<file_entry> file_ent, path_type const& path) override;
    int read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<file_entry> file_ent) override;
    int mknod(shared_ptr<file_entry>, path_type const&, mode_t, dev_t) override;
    int rmnod(shared_ptr<file_entry>, path_type const&) override;
    static bool is_matching_path(path_type const&);
};


}   // namespace planet

#endif  // PLANET_PACKET_SOCKET_HPP
