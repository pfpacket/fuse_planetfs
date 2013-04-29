#ifndef PLANET_OPERATION_LAYER
#define PLANET_OPERATION_LAYER

#include <planet/common.hpp>
#include <planet/planet_handle.hpp>
#include <planet/fs_core.hpp>

namespace planet {


    int read(handle_t handle, char *buf, size_t size, off_t offset)
    {
        auto& op_tuple = handle_mgr.get_operation_entry(handle);
        return std::get<1>(op_tuple)->read(
            std::get<0>(op_tuple), buf, size, offset
        );
    }

    int write(handle_t handle, char const *buf, size_t size, off_t offset)
    {
        return 0;
    }

    int close(handle_t handle)
    {
        return 0;
    }


}   // namespace planet

#endif  // PLANET_OPERATION_LAYER
