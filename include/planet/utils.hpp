#ifndef PLANET_UTILS_HPP
#define PLANET_UTILS_HPP

#include <planet/common.hpp>
#include <planet/fs_core.hpp>

namespace planet {

    // Cast entry to file_entry
    // Exception: If `entry` is not a type of file_entry
    //          : Also shared_ptr may throw exceptions
    shared_ptr<file_entry> file_cast(shared_ptr<fs_entry> entry);

    // Cast entry to dentry
    // Exception: If `entry` is not a type of dentry
    //          : Also shared_ptr may throw exceptions
    shared_ptr<dentry> directory_cast(shared_ptr<fs_entry> entry);

    template<typename Type, typename VecType>
    void store_data_to_vector(std::vector<VecType>& buffer, Type const& data)
    {
        buffer.reserve(sizeof (Type));
        *reinterpret_cast<Type *>(buffer.data()) = data;
    }

    template<typename Type, typename VecType>
    Type& get_data_from_vector(std::vector<VecType>& buffer)
    {
        if (buffer.capacity() < sizeof (Type))
            throw std::out_of_range(
                "get_data_from_vector(): buffer.capacity() < sizeof (Type)"
            );
        return *reinterpret_cast<Type *>(buffer.data());
    }


}   // namespace planet

#endif  // PLANET_UTILS_HPP
