
#include <planet/common.hpp>
#include <planet/eth/dir_op.hpp>
#include <net/if.h>

namespace planet {
namespace net {
namespace eth {


    shared_ptr<entry_operation> dir_op::new_instance() const
    {
        return std::make_shared<dir_op>();
    }

    int dir_op::open(shared_ptr<fs_entry> dir_ent, path_type const& path)
    {
        return 0;
    }

    int dir_op::read(shared_ptr<fs_entry> dir_ent, char *buf, size_t size, off_t offset)
    {
        return 0;
    }

    int dir_op::write(shared_ptr<fs_entry> dir_ent, char const *buf, size_t size, off_t offset)
    {
        return 0;
    }

    int dir_op::release(shared_ptr<fs_entry> dir_ent)
    {
        return 0;
    }

    int dir_op::mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t)
    {
        ::syslog(LOG_NOTICE, "%s: called", __PRETTY_FUNCTION__);
        return 0;
    }

    int dir_op::rmnod(shared_ptr<fs_entry>, path_type const&)
    {
        ::syslog(LOG_NOTICE, "%s: called", __PRETTY_FUNCTION__);
        throw exception_errno(EPERM);
        return -EPERM;
    }

    bool dir_op::is_matching_path(path_type const& path, file_type type)
    {
        return type == file_type::directory && path == "/eth";
    }


}   // namespace eth
}   // namespace net
}   // namespace planet
