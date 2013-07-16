
#include <planet/common.hpp>
#include <planet/net/tcp/address_op.hpp>
#include <planet/utils.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <sys/socket.h>

namespace planet {
namespace net {
namespace tcp {


    //
    // local_op - /tcp/*/local
    //
    void local_op::update_address(int sock, shared_ptr<fs_entry> file)
    {
        struct sockaddr_storage peer = {};
        socklen_t len = sizeof (peer);
        if (getsockname(sock, (sockaddr *)&peer, &len) < 0)
            throw exception_errno(errno, "getpeername: ", str(format(": fd=%1%") % sock).c_str());
        std::string hostname, servname;
        get_name_info((sockaddr *)&peer, len, hostname, servname);
        auto local_addr = str(format("%1%!%2%") % hostname % servname);
        file_cast(file)->data().clear();
        this->write(file, local_addr.c_str(), local_addr.length(), 0);
    }

    shared_ptr<fs_operation> local_op::new_instance()
    {
        return std::make_shared<local_op>(fs_root_);
    }

    int local_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        int ret = 0;
        ::syslog(LOG_INFO, "%s: %s", __PRETTY_FUNCTION__, path.parent_path().string().c_str());
        auto data_file = fs_root_.get_entry_of(path.string());
        if (auto sock = detail::fdtable.find_from_path(path.string()))
            update_address(*sock, data_file);
        else
            ret = -ENOTCONN;
        return ret;
    }

    bool local_op::is_matching_path(path_type const& path, file_type type)
    {
        return type == file_type::regular_file &&
            boost::regex_match(path.string(), path_reg::local);
    }

    //
    // remote_op - /tcp/*/remote
    //
    void remote_op::update_address(int sock, shared_ptr<fs_entry> file)
    {
        struct sockaddr_storage peer = {};
        socklen_t len = sizeof (peer);
        if (getpeername(sock, (sockaddr *)&peer, &len) < 0)
            throw exception_errno(errno, "getpeername: ", str(format(": fd=%1%") % sock).c_str());
        std::string hostname, servname;
        get_name_info((sockaddr *)&peer, len, hostname, servname);
        auto local_addr = str(format("%1%!%2%") % hostname % servname);
        file_cast(file)->data().clear();
        this->write(file, local_addr.c_str(), local_addr.length(), 0);
    }

    shared_ptr<fs_operation> remote_op::new_instance()
    {
        return std::make_shared<remote_op>(fs_root_);
    }

    int remote_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        int ret = 0;
        ::syslog(LOG_INFO, "%s: %s", __PRETTY_FUNCTION__, path.parent_path().string().c_str());
        auto data_file = fs_root_.get_entry_of(path.string());
        if (auto sock = detail::fdtable.find_from_path(path.string()))
            update_address(*sock, data_file);
        else
            ret = -ENOTCONN;
        return ret;
    }

    bool remote_op::is_matching_path(path_type const& path, file_type type)
    {
        return type == file_type::regular_file &&
            boost::regex_match(path.string(), path_reg::remote);
    }

}   // namespace tcp
}   // namespace net
}   // namespace planet
