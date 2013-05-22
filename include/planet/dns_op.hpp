#ifndef PLANET_DNS_OP_HPP
#define PLANET_DNS_OP_HPP

#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {


class dns_op : public planet_operation {
private:
    std::string hostname_;
    std::vector<std::string> resolved_names_;

    static int forward_lookup(
        string_type const& hostname, int family, std::vector<string_type>& store
    );

public:
    //dns_op() = default;
    dns_op()
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }
    ~dns_op() noexcept
    {
        ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
    }

    shared_ptr<planet_operation> new_instance() const;
    int open(shared_ptr<file_entry> file_ent, path_type const& path) override;
    int read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<file_entry> file_ent) override;
    int mknod(shared_ptr<file_entry>, path_type const&, mode_t, dev_t) override;
    int rmnod(shared_ptr<file_entry>, path_type const&) override;
    static bool is_matching_path(path_type const&);
};


}   // namespace planet

#endif  // PLANET_DNS_OP_HPP
