#ifndef PLANET_FILESYSTEM_HPP
#define PLANET_FILESYSTEM_HPP

#include <planet/common.hpp>
#include <planet/fs_core.hpp>

namespace planet {


    class filesystem {
    private:
        shared_ptr<path_manager>        path_mgr_;
        shared_ptr<operation_manager>   ops_mgr_;
        shared_ptr<core_file_system>    root_;

    public:
        typedef path_manager::priority priority;

        template<typename ...Types>
        filesystem(Types&& ...args)
            : path_mgr_{std::make_shared<path_manager>()},
              ops_mgr_{std::make_shared<operation_manager>()}
        {
            std::weak_ptr<path_manager>      path_mgr = path_mgr_;
            std::weak_ptr<operation_manager> ops_mgr  = ops_mgr_;
            root_ = core_file_system::create(
                std::forward<Types>(args)..., path_mgr, ops_mgr
            );
        }

        ~filesystem()
        {
            // Destroy them before
            //  destructing core_file_system
            path_mgr_->clear();
            ops_mgr_->clear();
            ::syslog(LOG_NOTICE, "filesystem: dtor: detroyed path_mgr_ and ops_mgr_");
            ::syslog(LOG_NOTICE, "filesystem: dtor: core_file_system: use_count=%ld", root_.use_count());
        }

        shared_ptr<core_file_system> root()
        {
            return root_;
        }
    };


}   // namespace planet

#endif  // PLANET_FILESYSTEM_HPP
