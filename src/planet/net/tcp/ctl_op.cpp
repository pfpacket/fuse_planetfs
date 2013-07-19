
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
#include <planet/operation_layer.hpp>
#include <planet/net/tcp/ctl_op.hpp>
#include <planet/utils.hpp>
#include <syslog.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

namespace planet {
namespace net {
namespace tcp {


    boost::regex reg_connect_req {R"(^connect ((\d{1,3}\.){3}\d{1,3})\!(\d{1,6}))"};

    shared_ptr<fs_operation> ctl_op::new_instance()
    {
        return std::make_shared<ctl_op>(fs_root_);
    }

    int ctl_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        boost::smatch m;
        boost::regex_search(path.string(), m, path_reg::ctl);
        current_fd_ = lexical_cast<int>(m[1]);
        return 0;
    }

    int ctl_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        int ret = 0;
        if (!fd_number_already_read_) {
            std::string dir_number = str(format("%1%") % current_fd_);
            if (dir_number.length() >= size)
                return -ENOBUFS;
            std::copy(dir_number.begin(), dir_number.end(), buf);
            buf[dir_number.length()] = '\0';
            ret = dir_number.length() + 1;
            fd_number_already_read_ = true;
        }
        return ret;
    }

    int ctl_op::interpret_request(string_type const& request)
    {
        boost::smatch m;
        int ret = -ENOTCONN;
        auto socket = detail::fdtable.find(lexical_cast<string_type>(current_fd_));
        if (boost::regex_search(request, boost::regex("^is_connected"))) {
            if (socket && sock_is_connected(*socket))
                ret = 0;
        } else if (boost::regex_search(request, m, reg_connect_req)) {
            ::syslog(LOG_NOTICE, "%s: connecting to %s!%s", __PRETTY_FUNCTION__, m[1].str().c_str(), m[3].str().c_str());
            int new_sock = sock_connect_to(m[1], m[3]);
            detail::fdtable.insert(lexical_cast<string_type>(current_fd_), new_sock);
            ret = 0;
            ::syslog(LOG_NOTICE, "%s: connected to %s!%s", __PRETTY_FUNCTION__, m[1].str().c_str(), m[3].str().c_str());
        } else if (boost::regex_search(request, boost::regex("^hangup"))) {
            detail::fdtable.erase(lexical_cast<string_type>(current_fd_));
            ret = 0;
        } else if (boost::regex_search(request, boost::regex{R"(^keepalive(|( (\d+))))"})) {
            if (socket) {
                int optvalue = 1;
                if (::setsockopt(*socket, SOL_SOCKET, SO_KEEPALIVE, &optvalue, sizeof (optvalue)) < 0)
                    throw exception_errno(errno);
                if (m[3].length()) {
                    optvalue = lexical_cast<int>(m[3]);
                    if (::setsockopt(*socket, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&optvalue, sizeof(optvalue)) < 0)
                        throw exception_errno(errno);
                }
                ret = 0;
            }
        } else
            ret = -ENOTSUP;
        return ret;
    }

    int ctl_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        std::string request(buf, size);
        int result = interpret_request(request);
        return !result ? size : result;
    }

    int ctl_op::release(shared_ptr<fs_entry> file_ent)
    {
        return 0;
    }

    int ctl_op::mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t, dev_t)
    {
        return 0;
    }

    int ctl_op::rmnod(shared_ptr<fs_entry> file_ent, path_type const&)
    {
        return -EPERM;
    }

    bool ctl_op::is_matching_path(path_type const& path, file_type type)
    {
        return
            type == file_type::regular_file
            && boost::regex_match(
                path.string(),
                path_reg::ctl
            );

    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
