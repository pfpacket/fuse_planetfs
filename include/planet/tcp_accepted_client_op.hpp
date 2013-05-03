#ifndef PLANET_TCP_ACCEPTED_CLIENT_HPP
#define PLANET_TCP_ACCEPTED_CLIENT_HPP

#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {


class tcp_accepted_client_op : public planet_operation {
private:
    int fd_;

public:
    static char const host_port_delimiter = '!';

    tcp_accepted_client_op() = default;

    shared_ptr<planet_operation> new_instance() const;
    int open(shared_ptr<file_entry> file_ent, path_type const& path) override;
    int read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<file_entry> file_ent) override;
    static bool is_matching_path(path_type const&);
};


}   // namespace planet

#endif  // PLANET_TCP_ACCEPTED_CLIENT_HPP
