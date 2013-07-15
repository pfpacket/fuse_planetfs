#ifndef PLANET_ETH_DIR_OP_HPP
#define PLANET_ETH_DIR_OP_HPP

#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace eth {


class dir_op final : public fs_operation {
private:
    core_file_system& fs_root_;
public:
    dir_op(core_file_system& fs_root) : fs_root_(fs_root)
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }

    ~dir_op()
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }

    shared_ptr<fs_operation> new_instance() override;
    int mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t) override;
    int rmnod(shared_ptr<fs_entry>, path_type const&) override;
    static bool is_matching_path(path_type const&, file_type);
};


}   // namespace eth
}   // namespace net
}   // namespace planet

#endif  // PLANET_ETH_DIR_OP_HPP
