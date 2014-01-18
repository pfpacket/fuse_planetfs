
#include <planet/common.hpp>

namespace planet {


    namespace detail {

        generic_shared_null_ptr_t const shared_null_ptr;

    }   // namespace detail

    void throw_system_error(int err)
    {
        throw std::system_error(err, std::system_category());
    }

    void throw_system_error(int err, std::string const& errmsg)
    {
        throw std::system_error(err, std::system_category(), errmsg);
    }


}   // namespace planet
