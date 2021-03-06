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

    class handle_manager {
    public:
        typedef std::tuple<
            shared_ptr<entry_op>,
            shared_ptr<fs_entry>
        > entry_type;
    private:
        std::mutex mtx_;
        typedef std::lock_guard<decltype(mtx_)> lock_guard;
        handle_t current_;
        std::map<handle_t, entry_type> ops_;

    public:
        handle_manager(int init = 0)
            : current_(init)
        {
        }

        entry_type const& get_op_entry(handle_t ph)
        try {
            return ops_.at(ph);
        } catch (...) {
            throw std::invalid_argument(
                str(format("handle_manager: Invalid handle: %1%") % ph)
            );
        }

        template<typename ...Types>
        handle_t register_op(shared_ptr<entry_op> op, shared_ptr<fs_entry> fp)
        {
            lock_guard lock(mtx_);
            ops_.insert(
                std::make_pair(
                    current_ + 1,
                    std::make_tuple(op, fp)
                )
            );
            return ++current_;
        }

        void unregister_op(handle_t ph)
        {
            lock_guard lock(mtx_);
            ops_.erase(ph);
        }
    };

    extern handle_t get_handle_from(struct fuse_file_info const& fi);
    extern void set_handle_to(struct fuse_file_info& fi, handle_t ph);


}   // namespace planet

#endif  // PLANET_SOCKET_HPP
