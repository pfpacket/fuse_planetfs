
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/net/tcp/dir_op.hpp>
#include <net/if.h>

namespace planet {
namespace net {
namespace tcp {


    shared_ptr<entry_op> dir_op::create_op()
    {
        return std::make_shared<dir_op>(fs_root_);
    }

    int dir_op::mknod(shared_ptr<fs_entry>, path_type const& path, mode_t, dev_t)
    {
        ::syslog(LOG_NOTICE, "%s: dir=%s", __PRETTY_FUNCTION__, path.string().c_str());
        return fs_root_->mknod("/tcp/clone", S_IRUSR | S_IWUSR, 0);
    }

    int dir_op::rmnod(shared_ptr<fs_entry>, path_type const&)
    {
        ::syslog(LOG_NOTICE, "%s: called", __PRETTY_FUNCTION__);
        throw_system_error(EPERM);
        return -EPERM;
    }

    bool dir_op::match_path(path_type const& path, file_type type)
    {
        return type == file_type::directory && path == "/tcp";
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
