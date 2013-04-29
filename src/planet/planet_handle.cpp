
#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/planet_handle.hpp>

namespace planet {


inline planet_handle_manager::planet_handle_manager(int init) : current_(init)
{
}

void planet_handle_manager::unregister_op(handle_t ph)
{
    lock_guard lock(mtx_);
    ops_.erase(ph);
}

// Global planet handle manager
planet_handle_manager handle_mgr;
planet_path_manager path_mgr;

handle_t get_handle_from(struct fuse_file_info const& fi)
{
    return static_cast<handle_t>(fi.fh);
}

void set_handle_to(struct fuse_file_info& fi, handle_t ph)
{
    fi.fh = static_cast<decltype(fi.fh)>(ph);
}


}   // namespace planet
