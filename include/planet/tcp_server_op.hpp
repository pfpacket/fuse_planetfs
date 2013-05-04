#ifndef PLANET_TCP_SERVER_OP_HPP
#define PLANET_TCP_SERVER_OP_HPP

#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {


class tcp_server_op : public planet_operation {
private:
    int port_, fd_;
    std::string host_;
    core_file_system& fs_root_;

    static int do_tcp_server_open(std::string const& host, int port);

public:
    static char const host_port_delimiter = '!';

    tcp_server_op(core_file_system& fs_root) : fs_root_(fs_root)
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }
    ~tcp_server_op() noexcept
    {
        ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
    }

    shared_ptr<planet_operation> new_instance() const;
    int open(shared_ptr<file_entry> file_ent, path_type const& path) override;
    int read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<file_entry> file_ent) override;
    static bool is_matching_path(path_type const&);
};


}   // namespace planet

#endif  // PLANET_TCP_SERVER_OP_HPP
