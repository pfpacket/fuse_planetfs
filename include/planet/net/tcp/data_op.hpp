#ifndef PLANET_TCP_DATA_OP_HPP
#define PLANET_TCP_DATA_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace tcp {


class data_op : public fs_operation {
private:
    shared_ptr<core_file_system> fs_root_;
    int socket_ = -1;

public:
    data_op(shared_ptr<core_file_system> fs_root)
        : fs_root_(fs_root)
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }

    ~data_op()
    {
        ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
    }

    shared_ptr<fs_operation> new_instance() override;
    int open(shared_ptr<fs_entry> fs_ent, path_type const& path) override;
    int read(shared_ptr<fs_entry> fs_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<fs_entry> fs_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<fs_entry> fs_ent) override;
    int mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t) override;
    int rmnod(shared_ptr<fs_entry>, path_type const&) override;
    static bool is_matching_path(path_type const&, file_type);
};


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_DATA_OP_HPP
