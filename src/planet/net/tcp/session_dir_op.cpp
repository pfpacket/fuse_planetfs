
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/net/tcp/session_dir_op.hpp>

namespace planet {
namespace net {
namespace tcp {


    shared_ptr<entry_op> session_dir_type::create_op(shared_ptr<core_file_system> fs_root)
    {
        return planet::detail::shared_null_ptr;
    }

    int session_dir_type::install(shared_ptr<core_file_system> fs)
    {
        fs_root_ = fs;
        return 0;
    }

    int session_dir_type::mknod(shared_ptr<fs_entry>, path_type const& path, mode_t, dev_t)
    {
        ::syslog(LOG_NOTICE, "%s: dir=%s", __PRETTY_FUNCTION__, path.string().c_str());
        xpv::regex_match(path.string(), path_reg::session_dir);
        fs_root_->mknod(path.string() + "/data", S_IRUSR | S_IWUSR, 0);
        fs_root_->mknod(path.string() + "/ctl", S_IRUSR | S_IWUSR, 0);
        fs_root_->mknod(path.string() + "/local", S_IRUSR | S_IWUSR, 0);
        fs_root_->mknod(path.string() + "/remote", S_IRUSR | S_IWUSR, 0);
        return 0;
    }

    int session_dir_type::rmnod(shared_ptr<fs_entry>, path_type const& path)
    {
        ::syslog(LOG_NOTICE, "%s: called", __PRETTY_FUNCTION__);
        detail::fdtable.erase_from_path(path.string());
        return 0;
    }

    bool session_dir_type::match_path(path_type const& path, file_type type)
    {
        return type == file_type::directory
            && xpv::regex_match(
                path.string(),
                path_reg::session_dir
            );
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
