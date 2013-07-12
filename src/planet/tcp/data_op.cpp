
#include <planet/common.hpp>
#include <planet/operation_layer.hpp>
#include <planet/tcp/data_op.hpp>
#include <planet/utils.hpp>
#include <syslog.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

namespace planet {
namespace net {
namespace tcp {


    shared_ptr<fs_operation> data_op::new_instance()
    {
        return std::make_shared<data_op>(fs_root_);
    }

    int create_socket()
    {
        int soc = ::socket(sock_arg::domain, sock_arg::type, sock_arg::protocol);
        if (soc < 0)
            throw exception_errno(errno);
        return soc;
    }

    int data_op::open(shared_ptr<fs_entry> fs_ent, path_type const& path)
    {
        socket_ = get_data_from_vector<int>(data_vector(*file_cast(fs_ent)));
        return 0;
    }

    int data_op::read(shared_ptr<fs_entry> fs_ent, char *buf, size_t size, off_t offset)
    {
        int bytes = ::recv(socket_, buf, size, 0);
        if (bytes < 0)
            throw exception_errno(errno);
        return bytes;
    }

    int data_op::write(shared_ptr<fs_entry> fs_ent, char const *buf, size_t size, off_t offset)
    {
        int bytes = ::send(socket_, buf, size, 0);
        if (bytes < 0)
            throw exception_errno(errno);
        return bytes;
    }

    int data_op::release(shared_ptr<fs_entry> fs_ent)
    {
        return 0;
    }

    int data_op::mknod(shared_ptr<fs_entry> fs_ent, path_type const& path, mode_t, dev_t)
    {
        int sock = create_socket();
        store_data_to_vector(data_vector(*file_cast(fs_ent)), sock);
        return 0;
    }

    int data_op::rmnod(shared_ptr<fs_entry> fs_ent, path_type const&)
    {
        if (socket_ >= 0)
            ::close(socket_);
        return -EPERM;
    }

    bool data_op::is_matching_path(path_type const& path, file_type type)
    {
        return
            type == file_type::regular_file
            && boost::regex_match(
                path.string(),
                boost::regex("/tcp/([0-9]+)/data")
            );

    }


}   // namespace tcp
}   // namespace net
}   // namespace planet

