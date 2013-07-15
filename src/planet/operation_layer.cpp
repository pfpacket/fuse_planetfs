
#include <planet/operation_layer.hpp>
#include <planet/fs_core.hpp>

namespace planet {


    int read(handle_t handle, char *buf, size_t size, off_t offset)
    {
        auto& op_tuple = handle_mgr.get_operation_entry(handle);
        return std::get<0>(op_tuple)->read(
            std::get<1>(op_tuple), buf, size, offset
        );
    }

    int write(handle_t handle, char const *buf, size_t size, off_t offset)
    {
        auto& op_tuple = handle_mgr.get_operation_entry(handle);
        return std::get<0>(op_tuple)->write(
            std::get<1>(op_tuple), buf, size, offset
        );
    }

    int close(handle_t handle)
    {
        auto& op_tuple = handle_mgr.get_operation_entry(handle);
        int ret = std::get<0>(op_tuple)->release(std::get<1>(op_tuple));
        handle_mgr.unregister_op(handle);
        return ret;
    }


}   // namespace planet
