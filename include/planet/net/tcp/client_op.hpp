#ifndef PLANET_TCP_CLIENT_OP_HPP
#define PLANET_TCP_CLIENT_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace tcp {


    class client_op : public entry_op {
    private:
        int fd_;

    public:

        //client_op() = default;
        client_op(shared_ptr<core_file_system>)
        {
        }

        ~client_op() noexcept
        {
        }

        int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
        int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
        int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
        int release(shared_ptr<fs_entry> file_ent) override;
        int poll(pollmask_t&);
    };

    class client_type : public file_ops_type {
    public:
        static const string_type type_name;
        client_type() : file_ops_type(type_name)
        {
        }

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system>) override
        {
            return std::make_shared<client_op>(::planet::detail::shared_null_ptr);
        }

        int mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t, dev_t) override
        {
            auto filename   = path.filename().string();
            auto pos        = filename.find_first_of(host_port_delimiter);
            auto host       = filename.substr(0, pos);
            auto port       = filename.substr(pos + 1);
            syslog(LOG_INFO, "client_op::mknod: connecting to host=%s, port=%s", host.c_str(), port.c_str());
            int sock = sock_connect_to(host, port);
            syslog(LOG_NOTICE, "client_op::mknod: connection established %s!%s fd=%d opened", host.c_str(), port.c_str(), sock);
            detail::fdtable.insert(path.string(), sock);
            return 0;
        }

        int rmnod(shared_ptr<fs_entry> file_ent, path_type const& path) override
        {
            detail::fdtable.erase(path.string());
            return 0;
        }

        bool match_path(path_type const& path, file_type type) override
        {
            return  type == file_type::regular_file &&
                    path.parent_path() == "/tcp" &&
                    path.filename().string()[0] != '*';
        }
    };


}   // namespace tcp
}   // namespace net
}   // namespace planet

#endif  // PLANET_TCP_CLIENT_OP_HPP
