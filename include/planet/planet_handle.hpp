#ifndef PLANET_SOCKET_HPP
#define PLANET_SOCKET_HPP

#include <planet/common.hpp>
#include <map>
#include <deque>
#include <mutex>
#include <memory>
#include <utility>
#include <cerrno>
#include <planet/basic_operation.hpp>

struct fuse_file_info;

namespace planet {


// planet handle type
typedef int handle_t;

class planet_handle_manager {
private:
    std::mutex mtx_;
    typedef std::lock_guard<decltype(mtx_)> lock_guard;
    handle_t current_;
    typedef std::tuple<
        shared_ptr<file_entry>,
        shared_ptr<planet_operation>
    > entry_type;
    std::map<handle_t, entry_type> ops_;

public:
    planet_handle_manager(int init = 0);

    entry_type const& get_operation_entry(handle_t ph)
    {
        return ops_.at(ph);
    }

    template<typename ...Types>
    handle_t register_op(shared_ptr<file_entry> fp, shared_ptr<planet_operation> op)
    {
        lock_guard lock(mtx_);
        ops_.insert(
            std::make_pair(
                current_ + 1,
                std::make_tuple(fp, op)
            )
        );
        return ++current_;
    }

    void unregister_op(handle_t ph);
};

extern planet_handle_manager handle_mgr;


handle_t get_handle_from(struct fuse_file_info const& fi);

void set_handle_to(struct fuse_file_info& fi, handle_t ph);

}   // namespace planet

#endif  // PLANET_SOCKET_HPP
