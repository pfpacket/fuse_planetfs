#ifndef PLANET_TCP_SESSION_DIR_HPP
#define PLANET_TCP_SESSION_DIR_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace tcp {


class session_dir_op final : public fs_operation {
private:
    core_file_system& fs_root_;

public:
    session_dir_op(core_file_system& fs_root) : fs_root_(fs_root)
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }

    ~session_dir_op()
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }

    shared_ptr<fs_operation> new_instance() override;
    int mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t) override;
    int rmnod(shared_ptr<fs_entry>, path_type const&) override;
    static bool is_matching_path(path_type const&, file_type);
};


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_SESSION_DIR_HPP
