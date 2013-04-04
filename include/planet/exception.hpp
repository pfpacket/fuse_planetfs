#ifndef PLANET_EXCEPTION_HPP
#define PLANET_EXCEPTION_HPP

#include <cerrno>
#include <string>
#include <stdexcept>

namespace planet {

class exception_errno : std::runtime_error {
public:
    exception_errno(int err, std::string const& msg)
        : std::runtime_error(msg), errno_(err)
    {
    }

    int get_errno() const
    {
        return errno_;
    }
private:
    int errno_;
};

}   // namespace planet

#endif  // PLANET_EXCEPTION_HPP
