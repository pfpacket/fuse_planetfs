#ifndef FUSECPP_CORE_HPP
#define FUSECPP_CORE_HPP

#include <fusecpp/common.hpp>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <boost/filesystem.hpp>

namespace fusecpp {


//
// fusecpp abstract base class
//    for every filesystem entry
//
class fusecpp_entry {
public:
    virtual ~fusecpp_entry() = default;
    // return if this entry is a file
    virtual bool is_file() const noexcept = 0;
    // return if this entry is a directory
    virtual bool is_directory() const noexcept = 0;
    // return the path of this entry
    virtual path_type const& path() const = 0;
};


//
// fusecpp file
//
class file : public fusecpp_entry {
private:
    mode_t mode_;
    path_type path_;
    std::vector<char> data_;
public:
    int private_data;
    file(path_type const& path, mode_t mode) : mode_(mode), path_(path) {}
    bool is_file() const noexcept override { return true; }
    bool is_directory() const noexcept override { return false; }
    mode_t get_mode() const { return mode_; }
    void set_mode(mode_t mode) { mode_ = mode; }
    path_type const& path() const { return path_; }
    auto data() -> decltype(data_)& { return data_; }
    auto data() const -> decltype(data_) const& { return data_; }
};


//
// fusecpp directory
//
class directory : public fusecpp_entry {
public: typedef shared_ptr<fusecpp_entry> value_type;
private:
    mode_t mode_;
    path_type path_;
    std::vector<value_type> entries_;
public:
    directory(path_type const& path, int mode = 0)
        : mode_(mode), path_(path) {}
    virtual ~directory() noexcept {}
    bool is_file() const noexcept override { return false; }
    bool is_directory() const noexcept override { return true; }
    path_type const& path() const { return path_; }
    auto entries() -> decltype(entries_)& { return entries_; }
    auto entries() const -> decltype(entries_) const& { return entries_; }

    // This is only for root directory
    shared_ptr<directory> get_this_ptr()
    {
        return shared_ptr<directory>(this, detail::null_deleter);
    }

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

    bool create_file(path_type const& path, mode_t mode = 0)
    {
        auto p = path.string();
        if (p.empty())
            return false;
        if (p[0] != '/')    // relative path
            p = this->path().string() + "/" + path.string();
        entries_.push_back(std::make_shared<file>(p, mode));
        return true;
    }

    bool create_directory(path_type const& path, mode_t mode = 0)
    {
        auto p = path.string();
        if (p.empty())
            return false;
        if (p[0] != '/')    // relative path
            p = this->path().string() + "/" + path.string();
        entries_.push_back(std::make_shared<directory>(p, mode));
        return true;
    }

    bool remove_entry(path_type const& path)
    {
        auto p = path.string();
        if (p.empty())
            return false;
        if (p[0] != '/')    // relative path
            p = this->path().string() + "/" + path.string();
        entries_.erase(
            std::remove_if(
                entries_.begin(), entries_.end(),
                [&path](value_type const& ptr){ return ptr->path() == path; }
            ),
            entries_.end()
        );
        return true;
    }
};


}   // namespace fusecpp

#endif  // FUSECPP_CORE_HPP
