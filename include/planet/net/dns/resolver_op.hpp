#ifndef PLANET_DNS_OP_HPP
#define PLANET_DNS_OP_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace dns {


class resolver_op : public fs_operation {
private:
    std::string hostname_;
    std::vector<std::string> resolved_names_;

    static int forward_lookup(
        string_type const& hostname, int family, std::vector<string_type>& store
    );

public:
    //resolver_op() = default;
    resolver_op()
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }
    ~resolver_op() noexcept
    {
        ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
    }

    shared_ptr<fs_operation> new_instance() override;
    int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
    int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<fs_entry> file_ent) override;
    int mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t) override;
    int rmnod(shared_ptr<fs_entry>, path_type const&) override;
    static bool is_matching_path(path_type const&, file_type);
};


}   // namespace dns
}   // namespace net
}   // namespace planet

#endif  // PLANET_DNS_OP_HPP