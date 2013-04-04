#ifndef FUSECPP_UTILITY_HPP
#define FUSECPP_UTILITY_HPP

//
// Utilities for fusecpp
//

#include <fusecpp/common.hpp>
#include <fusecpp/core.hpp>

namespace fusecpp {


// Get head pointer of data from file `f`
char *get_file_data(file& f);
char const *get_file_data(file const& f);

// Cast fusecpp_entry to file
shared_ptr<file> file_cast(directory::value_type const& ptr);

// Cast fusecpp_entry to directory
shared_ptr<directory> directory_cast(directory::value_type const& ptr);

// Search fusecpp_entry from `root`
shared_ptr<fusecpp_entry> search_entry(directory &root, path_type const& path);

// Search file entry which has the same path and return its pointer
shared_ptr<file> search_file(directory &root, path_type const& path);

// Search directory entry which has the same path and return its pointer
shared_ptr<directory> search_directory(directory &root, path_type const& path);

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
