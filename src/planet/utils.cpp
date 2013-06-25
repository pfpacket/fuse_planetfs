
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/utils.hpp>

namespace planet {


    shared_ptr<file_entry> file_cast(shared_ptr<fs_entry> entry)
    {
        if (!entry || entry->type() != file_type::regular_file)
            throw std::runtime_error(get_errmsg(detail::pe_file_cast));
        return std::static_pointer_cast<file_entry>(entry);
    }

    shared_ptr<dentry> directory_cast(shared_ptr<fs_entry> entry)
    {
        if (!entry || entry->type() != file_type::directory)
            throw std::runtime_error(get_errmsg(detail::pe_dir_cast));
        return std::static_pointer_cast<dentry>(entry);
    }

    shared_ptr<file_entry> search_file_entry(core_file_system const& root, path_type const& path)
    {
        auto entry = root.get_entry_of(path);
        return (entry->type() == file_type::regular_file ?
            file_cast(entry) : detail::shared_null_ptr);
    }

    shared_ptr<dentry> search_dir_entry(core_file_system const& root, path_type const& path)
    {
        auto entry = root.get_entry_of(path);
        return (entry->type() == file_type::directory ?
            directory_cast(entry) : detail::shared_null_ptr);
    }


}   // namespace planet
