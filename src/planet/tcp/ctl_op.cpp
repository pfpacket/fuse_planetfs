
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
#include <planet/operation_layer.hpp>
#include <planet/tcp/ctl_op.hpp>
#include <planet/utils.hpp>
#include <netinet/tcp.h>
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
        if (!boost::regex_match(path.string(), m, path_reg::ctl))
            throw std::runtime_error(
                __PRETTY_FUNCTION__ + std::string(": ") + path.string() + ": unexpected path"
            );
        current_fd_ = boost::lexical_cast<int>(m[1]);
        auto data_file = fs_root_.get_entry_of(path.parent_path().string() + "/data");
        socket_ = get_data_from_vector<int>(data_vector(*file_cast(data_file)));
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

    void sock_connect_to(int fd, std::string const& host, int port)
    {
        struct sockaddr_in sin = {AF_INET, htons(port), {0}, {0}};
        inet_pton(AF_INET, host.c_str(), &sin.sin_addr);
        if (connect(fd, reinterpret_cast<struct sockaddr *>(&sin), sizeof(sin)) < 0)
            throw planet::exception_errno(errno);
    }

    // getsockopt(TCP_INFO) depends on Linux functionality
    bool sock_is_connected(int sock)
    {
        struct tcp_info info;
        socklen_t info_length = sizeof (info);
        if (getsockopt(sock, SOL_TCP, TCP_INFO, &info, &info_length) != 0)
            throw exception_errno(errno);
        return info.tcpi_state == TCP_ESTABLISHED;
    }

    int ctl_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        int ret = size;
        boost::smatch m;
        std::string request(buf, size);
        if (boost::regex_search(request, boost::regex("^is_connected"))) {
            if (!sock_is_connected(socket_)) {
                ret = -ECONNREFUSED;
                ::shutdown(socket_, SHUT_RDWR);
            }
        } else if (boost::regex_search(request, m, reg_connect_req))
            sock_connect_to(socket_, m[1], boost::lexical_cast<int>(m[3]));
        else
            ret = -ENOTSUP;
        return ret;
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
