
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
#include <planet/operation_layer.hpp>
#include <planet/tcp/ctl_op.hpp>
#include <planet/utils.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

namespace planet {
namespace net {
namespace tcp {


    shared_ptr<fs_operation> ctl_op::new_instance()
    {
        return std::make_shared<ctl_op>(fs_root_);
    }

    int ctl_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        boost::smatch m;
        if (!boost::regex_match(path.string(), m, boost::regex("/tcp/([0-9]+)/ctl")))
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

    void connect_to(int fd, std::string const& host, int port)
    {
        struct sockaddr_in sin = {AF_INET, htons(port), {0}, {0}};
        inet_pton(AF_INET, host.c_str(), &sin.sin_addr);
        if (connect(fd, reinterpret_cast<struct sockaddr *>(&sin), sizeof(sin)) < 0)
            throw planet::exception_errno(errno);
    }

    int ctl_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        boost::smatch m;
        std::string request(buf, size);
        if (boost::regex_search(request, boost::regex("^is_connected")))
            return -ECONNREFUSED;
        else if (boost::regex_search(request, m, boost::regex(R"(^connect ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)\!([0-9]+))")))
            connect_to(socket_, m[1], boost::lexical_cast<int>(m[2]));
        else
            return -ENOTSUP;
        return size;
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
                boost::regex("/tcp/([0-9]+)/ctl")
            );

    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
