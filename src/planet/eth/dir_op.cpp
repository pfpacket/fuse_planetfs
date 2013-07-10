
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/eth/dir_op.hpp>
#include <net/if.h>

namespace planet {
namespace net {
namespace eth {


    shared_ptr<fs_operation> dir_op::new_instance()
    {
        return std::make_shared<dir_op>(fs_root_);
    }

    int dir_op::mknod(shared_ptr<fs_entry>, path_type const& path, mode_t, dev_t)
    {
        ::syslog(LOG_NOTICE, "%s: path=%s", __PRETTY_FUNCTION__, path.string().c_str());
        auto ifnames = make_unique_ptr(
            if_nameindex(), [](struct if_nameindex *p){ if_freenameindex(p); });
        if (!ifnames)
            throw exception_errno(errno);
        for (auto *i = ifnames.get(); !(i->if_index == 0 && i->if_name == NULL); i++)
            fs_root_.mknod(path.string() + "/" + i->if_name, S_IRUSR | S_IWUSR, 0);
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
        return type == file_type::directory && (path == "/eth" || path == "/ip");
    }


}   // namespace eth
}   // namespace net
}   // namespace planet
