
#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/handle.hpp>
#include <fuse/fuse.h>

namespace planet {


    // Global planet handle manager
    handle_manager handle_mgr;

    handle_t get_handle_from(struct fuse_file_info const& fi)
    {
        return static_cast<handle_t>(fi.fh);
    }

    void set_handle_to(struct fuse_file_info& fi, handle_t ph)
    {
        fi.fh = static_cast<decltype(fi.fh)>(ph);
    }


}   // namespace planet
