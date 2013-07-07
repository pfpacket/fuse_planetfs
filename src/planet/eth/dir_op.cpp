
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/eth/dir_op.hpp>
#include <net/if.h>

namespace planet {
namespace net {
namespace eth {


    shared_ptr<fs_operation> dir_op::new_instance() const
    {
        return std::make_shared<dir_op>(fs_root_);
    }

    int dir_op::mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t)
    {
        ::syslog(LOG_NOTICE, "%s: called", __PRETTY_FUNCTION__);
        struct if_nameindex *if_ni = if_nameindex(), *i;
        if (!if_ni)
            throw exception_errno(errno);
        auto ifnames
            = make_unique_ptr(if_ni, [](struct if_nameindex *p){ if_freenameindex(p); });
        for (i = ifnames.get(); !(i->if_index == 0 && i->if_name == NULL); i++)
            fs_root_.mknod(std::string("/eth/") + i->if_name, S_IRUSR | S_IWUSR, 0);
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
