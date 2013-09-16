#ifndef PLANET_EXCEPTION_HPP
#define PLANET_EXCEPTION_HPP

#include <cerrno>
#include <string>
#include <cstring>
#include <stdexcept>

namespace planet {

class exception_errno : public std::exception {
public:
    exception_errno(int err, string_type const& pre = "", string_type const& post = "")
        : errno_(err), msg_{pre + std::strerror(err) + post}
    {
    }

    virtual ~exception_errno() = default;

    int get_errno() const noexcept
    {
        return errno_;
    }

    char const *what() const noexcept
    {
        return msg_.c_str();
    }

private:
    int errno_;
    std::string msg_;
};

}   // namespace planet

#endif  // PLANET_EXCEPTION_HPP
