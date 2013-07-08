#ifndef PLANET_EXCEPTION_HPP
#define PLANET_EXCEPTION_HPP

#include <cerrno>
#include <string>
#include <cstring>
#include <stdexcept>

namespace planet {

class exception_errno : public std::exception {
public:
    exception_errno(int err) : errno_(err)
    {
    }

    virtual ~exception_errno() = default;

    int get_errno() const noexcept
    {
        return errno_;
    }

    char const *what() const noexcept
    {
        return std::strerror(errno_);
    }

private:
    int errno_;
};

}   // namespace planet

#endif  // PLANET_EXCEPTION_HPP
