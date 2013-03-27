#ifndef FUSECPP_CORE_HPP
#define FUSECPP_CORE_HPP

#include <vector>
#include <string>
#include <list>
#include <map>
#include <memory>
#include <boost/filesystem.hpp>

namespace fusecpp {

// fusecpp path type (better string class)
typedef boost::filesystem::path path_type;

// fusecpp shared_ptr
template<typename T>
using shared_ptr = std::shared_ptr<T>;


namespace detail {
    static auto const null_deleter = [](void *) {};
    static char const * const errmsg[] = {
        "invalid cast from \'fusecpp_entry\' to \'file\'",
        "invalid cast from \'fusecpp_entry\' to \'directory\'"
    };
    enum errmsg_number {fe_file_cast = 0, fe_dir_cast};
}   // namespace detail

char const *get_errmsg(detail::errmsg_number num) noexcept
{
    return detail::errmsg[num];
}


//
// fusecpp abstract base class
//    for every filesystem entry
//
class fusecpp_entry {
public:
    virtual ~fusecpp_entry() = default;
    // return if this entry is a file
    virtual bool is_file() const = 0;
    // return if this entry is a directory
    virtual bool is_directory() const = 0;
    // return the path of this entry
    virtual path_type const& path() const = 0;
};


//
// fusecpp file
//
class file : public fusecpp_entry {
private:
    path_type path_;
    std::vector<char> data_;
public:
    file(path_type const& path) : path_(path) {}
    bool is_file() const { return true; }
    bool is_directory() const { return false; }
    path_type const& path() const { return path_; }
    auto get_data() -> decltype(data_)& { return data_; }
    auto get_data() const -> const decltype(data_)& { return data_; }
};


//
// fusecpp directory
//
class directory : public fusecpp_entry {
public: typedef shared_ptr<fusecpp_entry> value_type;
private:
    int mode_;
    path_type path_;
    std::list<value_type> entries_;
public:
    directory(path_type const& path, int mode = 0)
        : mode_(mode), path_(path) {}
    bool is_file() const { return false; }
    bool is_directory() const { return true; }
    path_type const& path() const { return path_; }
    auto get_entries() -> decltype(entries_)& { return entries_; }
    auto get_entries() const -> const decltype(entries_)& { return entries_; }

    bool add_file(file const& f)
    {
        entries_.push_back(std::make_shared<file>(f));
        return true;
    }

    bool add_directory(directory const& d)
    {
        entries_.push_back(std::make_shared<directory>(d));
        return true;
    }

    bool create_file(path_type const& path)
    {
        entries_.push_back(std::make_shared<file>(path));
        return true;
    }

    bool create_directory(path_type const& path)
    {
        entries_.push_back(std::make_shared<directory>(path));
        return true;
    }
};


// Cast fusecpp_entry to file
shared_ptr<file> file_cast(directory::value_type ptr)
{
    if (!ptr->is_file())
        throw std::runtime_error(get_errmsg(detail::fe_file_cast));
    return shared_ptr<file>(static_cast<file *>(ptr.get()), detail::null_deleter);
}


// Cast fusecpp_entry to directory
shared_ptr<directory> directory_cast(directory::value_type ptr)
{
    if (!ptr->is_directory())
        throw std::runtime_error(get_errmsg(detail::fe_file_cast));
    return shared_ptr<directory>(static_cast<directory *>(ptr.get()), detail::null_deleter);
}


// Search file entry which has the same path and return its pointer
shared_ptr<file> search_file(directory &root, path_type const& path)
{
    for (auto ptr : root.get_entries()) {
        if (ptr->is_file() && ptr->path() == path)
            return file_cast(ptr);
        else if (ptr->is_directory())
            if (auto ret_ptr = search_file(*directory_cast(ptr), path))
                return ret_ptr;
    }
    return shared_ptr<file>{};
}


// Search directory entry which has the same path and return its pointer
shared_ptr<directory> search_directory(directory &root, path_type const& path)
{
    if (root.path() == path)
        return shared_ptr<directory>{&root, detail::null_deleter};
    for (auto ptr : root.get_entries()) {
        if (ptr->is_directory()) {
            if (ptr->path() == path)
                return directory_cast(ptr);
            if (auto ret_ptr = search_directory(*directory_cast(ptr), path))
                return ret_ptr;
        }
    }
    return shared_ptr<directory>{};
}


}   // namespace fusecpp

#endif  // FUSECPP_CORE_HPP
