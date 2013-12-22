#ifndef PLANET_FUSE_POLLER_HPP
#define PLANET_FUSE_POLLER_HPP

#include <planet/common.hpp>
#include <atomic>
#include <tuple>
#include <algorithm>
#include <thread>
#include <mutex>
#include <planet/handle.hpp>

namespace planet {


    class fuse_poller {
    private:
        std::atomic<bool> poll_end_{false};
        std::thread polling_thread_;
        typedef std::tuple<
            handle_t, fuse_pollhandle *, pollmask_t
        > poll_info;
        std::vector<poll_info> phs_;
        mutable std::recursive_mutex phs_mtx_;
        typedef std::lock_guard<decltype(phs_mtx_)> lock_guard;

    public:
        fuse_poller();

        ~fuse_poller();

        bool register_new(handle_t handle, fuse_pollhandle *ph);
        bool unregister(handle_t handle);

        pollmask_t get_status(handle_t handle) const;

        void poll(shared_ptr<core_file_system>);

    private:
        void polling(shared_ptr<core_file_system>) noexcept;

        void poll_handles(shared_ptr<core_file_system>);
    };


}   // namespace planet

#endif  // PLANET_FUSE_POLLER_HPP
