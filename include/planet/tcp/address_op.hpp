#ifndef PLANET_TCP_ADDRESS_OP_HPP
#define PLANET_TCP_ADDRESS_OP_HPP

#include <planet/common.hpp>
#include <planet/tcp/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace tcp {


class local_op : public default_file_op {
private:
    core_file_system& fs_root_;

public:
    local_op(core_file_system& fs_root)
        :   fs_root_(fs_root)
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }

    ~local_op()
    {
        ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
    }

    void update_address(int, shared_ptr<fs_entry>);

    shared_ptr<fs_operation> new_instance() override;
    int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
    // int read(shared_ptr<fs_entry>, char *buf, size_t size, off_t offset) override;
    // int write(shared_ptr<fs_entry>, char const *buf, size_t size, off_t offset) override;
    static bool is_matching_path(path_type const&, file_type);
};

class remote_op : public default_file_op {
private:
    core_file_system& fs_root_;

public:
    remote_op(core_file_system& fs_root)
        :   fs_root_(fs_root)
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }

    ~remote_op()
    {
        ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
    }

    void update_address(int, shared_ptr<fs_entry>);

    shared_ptr<fs_operation> new_instance() override;
    int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
    // int read(shared_ptr<fs_entry>, char *buf, size_t size, off_t offset) override;
    // int write(shared_ptr<fs_entry>, char const *buf, size_t size, off_t offset) override;
    static bool is_matching_path(path_type const&, file_type);
};


}   // namespace tcp
}   // namespace net
}   // namespace planet


#endif  // PLANET_TCP_ADDRESS_OP_HPP
