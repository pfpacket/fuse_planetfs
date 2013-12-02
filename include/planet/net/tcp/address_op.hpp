#ifndef PLANET_TCP_ADDRESS_OP_HPP
#define PLANET_TCP_ADDRESS_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace tcp {


    //
    // /tcp/*/local
    //
    class local_op : public default_file_op {
    private:
        shared_ptr<core_file_system> fs_root_;

    public:
        local_op(shared_ptr<core_file_system> fs_root)
            :   default_file_op(fs_root), fs_root_(fs_root)
        {
            ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
        }

        ~local_op()
        {
            ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
        }

        void update_address(int, shared_ptr<fs_entry>);

        int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
        // int read(shared_ptr<fs_entry>, char *buf, size_t size, off_t offset) override;
        // int write(shared_ptr<fs_entry>, char const *buf, size_t size, off_t offset) override;
    };

    class local_type : public file_ops_type {
    public:
        local_type() : file_ops_type("planet.net.tcp.local")
        {
        }

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override
        {
            return std::make_shared<local_op>(fs_root);
        }

        bool match_path(path_type const& path, file_type type) override
        {
            return type == file_type::regular_file &&
                xpv::regex_match(path.string(), path_reg::local);
        }
    };

    //
    // /tcp/*/remote
    //
    class remote_op : public default_file_op {
    private:
        shared_ptr<core_file_system> fs_root_;

    public:
        remote_op(shared_ptr<core_file_system> fs_root)
            :   default_file_op(fs_root), fs_root_(fs_root)
        {
            ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
        }

        ~remote_op()
        {
            ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
        }

        void update_address(int, shared_ptr<fs_entry>);

        int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
        // int read(shared_ptr<fs_entry>, char *buf, size_t size, off_t offset) override;
        // int write(shared_ptr<fs_entry>, char const *buf, size_t size, off_t offset) override;
    };

    class remote_type : public file_ops_type {
    public:
        remote_type() : file_ops_type("planet.net.tcp.remote")
        {
        }

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override
        {
            return std::make_shared<remote_op>(fs_root);
        }

        bool match_path(path_type const& path, file_type type) override
        {
            return type == file_type::regular_file &&
                xpv::regex_match(path.string(), path_reg::remote);
        }
    };

}   // namespace tcp
}   // namespace net
}   // namespace planet


#endif  // PLANET_TCP_ADDRESS_OP_HPP
