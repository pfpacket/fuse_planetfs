#ifndef PLANET_ETH_RAW_OP_HPP 
#define PLANET_ETH_RAW_OP_HPP 

#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace eth {


class raw_op : public fs_operation {
private:
    int fd_;

    static void bind_to_interface(int fd, std::string const& ifname, int protocol);
    static int do_raw_open(int sock_type, int protocol, std::string const& ifname);

public:
    raw_op()
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }
    ~raw_op() noexcept
    {
        ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
    }

    shared_ptr<fs_operation> new_instance() override;
    int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
    int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<fs_entry> file_ent) override;
    int mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t) override;
    int rmnod(shared_ptr<fs_entry>, path_type const&) override;
    static bool is_matching_path(path_type const&, file_type);
};


}   // namespace eth
}   // namespace net
}   // namespace planet

#endif  // PLANET_ETH_RAW_OP_HPP 
