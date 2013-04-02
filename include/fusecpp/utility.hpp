#ifndef FUSECPP_UTILITY_HPP
#define FUSECPP_UTILITY_HPP

//
// Utilities for fusecpp
//

#include <fusecpp/common.hpp>
#include <fusecpp/core.hpp>

namespace fusecpp {


// Get head pointer of data from file `f`
char *get_file_data(file& f)
{
    return f.data().data();
}

char const *get_file_data(file const& f)
{
    return f.data().data();
}


// Cast fusecpp_entry to file
shared_ptr<file> file_cast(directory::value_type const& ptr)
{
    if (!ptr->is_file())
        throw std::runtime_error(get_errmsg(detail::fe_file_cast));
    return std::static_pointer_cast<file>(ptr);
}


// Cast fusecpp_entry to directory
shared_ptr<directory> directory_cast(directory::value_type const& ptr)
{
    if (!ptr->is_directory())
        throw std::runtime_error(get_errmsg(detail::fe_dir_cast));
    return std::static_pointer_cast<directory>(ptr);
}


// Search fusecpp_entry from `root`
shared_ptr<fusecpp_entry> search_entry(directory &root, path_type const& path)
{
    if (root.path() == path)
        return root.get_this_ptr();
    for (auto ptr : root.entries()) {
        if (ptr->path() == path)
            return ptr;
        if (ptr->is_directory())
            if (auto ret_ptr = search_entry(*directory_cast(ptr), path))
                return ret_ptr;
    }
    return detail::shared_null_ptr;
}


// Search file entry which has the same path and return its pointer
shared_ptr<file> search_file(directory &root, path_type const& path)
{
   if (auto ptr = search_entry(root, path))
       if (ptr->is_file())
           return file_cast(ptr);
   return detail::shared_null_ptr;
}


// Search directory entry which has the same path and return its pointer
shared_ptr<directory> search_directory(directory &root, path_type const& path)
{
    if (auto ptr = search_entry(root, path))
        if (ptr->is_directory())
            return directory_cast(ptr);
    return detail::shared_null_ptr;
}


/*bool recursive_create_dir(path_type const& path, mode_t mode = 0)
{
    directory *dir = this;
    for (std::size_t current_pos = 1; current_pos - 1 != std::string::npos; ) {
        auto pos = path.find_first_of('/', current_pos);
        std::string dirname = path.substr(current_pos, pos - current_pos);
        dir->create_directory(dirname);
        current_pos = pos + 1;
    }
}*/


}   // namespace fusecpp

#endif // FUSECPP_UTILITY_HPP
