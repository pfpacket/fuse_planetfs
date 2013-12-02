#ifndef PLANET_DNS_OP_HPP
#define PLANET_DNS_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace dns {


    class resolver_op : public default_file_op {
    private:
        std::string hostname_;
        std::vector<std::string> resolved_names_;

        static int forward_lookup(
            string_type const& hostname, int family, std::vector<string_type>& store
        );

    public:
        resolver_op() = default;
        resolver_op(shared_ptr<core_file_system>)
        {
            ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
        }

        ~resolver_op() noexcept
        {
            ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
        }

        int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
        int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
        int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
        int release(shared_ptr<fs_entry> file_ent) override;
    };

    class resolver_type : public file_ops_type {
    private:
    public:
        resolver_type() : file_ops_type("planet.net.dns.resolver")
        {
            ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
        }

        ~resolver_type() noexcept
        {
            ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
        }

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs) override
        {
            return make_shared<resolver_op>(fs);
        }

        bool match_path(path_type const& path, file_type type) override
        {
            return type == file_type::regular_file && path == "/dns";
        }
    };


}   // namespace dns
}   // namespace net
}   // namespace planet

#endif  // PLANET_DNS_OP_HPP
