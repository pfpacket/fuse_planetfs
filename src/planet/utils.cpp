
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/utils.hpp>

namespace planet {


    shared_ptr<file_entry> file_cast(shared_ptr<fs_entry> entry)
    {
        if (entry->type() != file_type::regular_file)
            throw std::runtime_error(get_errmsg(detail::pe_file_cast));
        return std::static_pointer_cast<file_entry>(entry);
    }

    shared_ptr<dentry> directory_cast(shared_ptr<fs_entry> entry)
    {
        if (entry->type() != file_type::directory)
            throw std::runtime_error(get_errmsg(detail::pe_dir_cast));
        return std::static_pointer_cast<dentry>(entry);
    }


}   // namespace planet
