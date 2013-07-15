
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/tcp/session_dir_op.hpp>
#include <boost/regex.hpp>

namespace planet {
namespace net {
namespace tcp {


    shared_ptr<fs_operation> session_dir_op::new_instance()
    {
        return std::make_shared<session_dir_op>(fs_root_);
    }

    int session_dir_op::mknod(shared_ptr<fs_entry>, path_type const& path, mode_t, dev_t)
    {
        ::syslog(LOG_NOTICE, "%s: dir=%s", __PRETTY_FUNCTION__, path.string().c_str());
        boost::smatch m;
        boost::regex_match(path.string(), m, path_reg::session_dir);
        fs_root_.mknod(path.string() + "/data", S_IRUSR | S_IWUSR, 0);
        fs_root_.mknod(path.string() + "/ctl", S_IRUSR | S_IWUSR, 0);
        fs_root_.mknod(path.string() + "/remote", S_IRUSR | S_IWUSR, 0);
        return 0;
    }

    int session_dir_op::rmnod(shared_ptr<fs_entry>, path_type const& path)
    {
        ::syslog(LOG_NOTICE, "%s: called", __PRETTY_FUNCTION__);
        detail::fdtable.erase_from_path(path.string());
        return 0;
    }

    bool session_dir_op::is_matching_path(path_type const& path, file_type type)
    {
        return type == file_type::directory
            && boost::regex_match(
                path.string(),
                path_reg::session_dir
            );
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
