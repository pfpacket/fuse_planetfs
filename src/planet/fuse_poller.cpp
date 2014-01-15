
#include <planet/common.hpp>
#include <planet/fuse_poller.hpp>
#include <planet/utils.hpp>
#include <planet/fs_core.hpp>
#include <chrono>

namespace planet {


    fuse_poller::fuse_poller()
    {
    }

    fuse_poller::~fuse_poller()
    {
        poll_end_.store(true);
        if (polling_thread_.joinable())
            polling_thread_.join();
    }

    bool fuse_poller::register_new(handle_t handle, fuse_pollhandle *ph)
    {
        lock_guard lock(phs_mtx_);
        auto it = std::find_if(phs_.begin(), phs_.end(),
            [handle](poll_info const& info) {
                return std::get<0>(info) == handle;
            }
        );
        // Insert a new handle or update a handle
        if (it == phs_.end())
            phs_.emplace_back(handle, ph, 0);
        else {
            if (std::get<1>(*it))
                ::fuse_pollhandle_destroy(std::get<1>(*it));
            std::get<1>(*it) = ph;
        }
        return it == phs_.end();
    }

    bool fuse_poller::unregister(handle_t handle)
    {
        lock_guard lock(phs_mtx_);
        auto it = std::remove_if(phs_.begin(), phs_.end(),
            [handle](poll_info const& info) {
                return std::get<0>(info) == handle;
            }
        );
        if (it != phs_.end())
            phs_.erase(it, phs_.end());
        return it != phs_.end();
    }

    pollmask_t fuse_poller::get_status(handle_t handle) const
    {
        lock_guard lock(phs_mtx_);
        auto it = std::find_if(phs_.begin(), phs_.end(),
            [handle](poll_info const& info) {
                return std::get<0>(info) == handle;
            }
        );
        return (it == phs_.end() ? 0 : std::get<2>(*it));
    }

    void fuse_poller::poll(shared_ptr<core_file_system> fs_root)
    {
        if (!polling_thread_.joinable())
            polling_thread_ = std::thread(&fuse_poller::polling, this, fs_root);
    }

    void fuse_poller::polling(shared_ptr<core_file_system> fs_root) noexcept
    {
        BOOST_LOG_TRIVIAL(trace) << "fuse_poller: polling: begin";
        while (!poll_end_.load()) {
            try {
                this->poll_handles(fs_root);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } catch (std::exception& e) {
                BOOST_LOG_TRIVIAL(error) << "poller: polling: Exception: " << e.what();
            } catch (...) {
            }
        }
        BOOST_LOG_TRIVIAL(trace) << "fuse_poller: polling: end";
    }

    void fuse_poller::poll_handles(shared_ptr<core_file_system> fs_root)
    {
        lock_guard lock(phs_mtx_);
        for (auto&& pollinfo : phs_) {
            std::get<2>(pollinfo) = 0;
            fs_root->poll(std::get<0>(pollinfo), std::get<2>(pollinfo));
            if (std::get<2>(pollinfo)) {
                ::fuse_notify_poll(std::get<1>(pollinfo));
                ::fuse_pollhandle_destroy(std::get<1>(pollinfo));
                std::get<1>(pollinfo) = nullptr;
            }
        }
    }


}   // namespace planet
