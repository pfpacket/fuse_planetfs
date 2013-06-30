#ifndef PLANET_ETH_DIR_OP_HPP
#define PLANET_ETH_DIR_OP_HPP

#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {
namespace net {
namespace eth {


class dir_op final : public entry_operation {
private:
public:
    dir_op()
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }

    ~dir_op()
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }

    shared_ptr<entry_operation> new_instance() const;
    int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
    int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<fs_entry> file_ent) override;
    int mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t) override;
    int rmnod(shared_ptr<fs_entry>, path_type const&) override;
    static bool is_matching_path(path_type const&, file_type);
};


}   // namespace eth
}   // namespace net
}   // namespace planet

#endif  // PLANET_ETH_DIR_OP_HPP
