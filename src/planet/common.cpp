
#include <planet/common.hpp>

namespace planet {

    namespace detail {

        char const * const errmsg[] = {
            "Invalid cast: not a file",
            "Invalid cast: not a directory"
        };

        generic_shared_null_ptr_t const shared_null_ptr;

    }   // namespace detail

    char const *get_errmsg(detail::errmsg_number num) noexcept
    {
        return num >= detail::pe_end ? nullptr : detail::errmsg[num];
    }

}   // namespace planet

