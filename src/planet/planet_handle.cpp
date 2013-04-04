
#include <planet/common.hpp>
#include <planet/planet_handle.hpp>

namespace planet {


inline planet_handle_manager::planet_handle_manager(int init) : current_(init)
{
}

shared_ptr<planet_operation> planet_handle_manager::operator[](planet_handle_t index)
{
    return ops_.at(index);
}

void planet_handle_manager::unregister_op(planet_handle_t ph)
{
    lock_guard lock(mtx_);
    ops_.erase(ph);
}

// Global planet handle manager
planet_handle_manager handle_mgr;

planet_handle_t get_handle_from(struct fuse_file_info const& fi)
{
    return static_cast<planet_handle_t>(fi.fh);
}

void set_handle_to(struct fuse_file_info& fi, planet_handle_t ph)
{
    fi.fh = static_cast<decltype(fi.fh)>(ph);
}


}   // namespace planet
