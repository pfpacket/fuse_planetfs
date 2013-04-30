
#include <planet/common.hpp>

namespace planet {

    namespace detail {
        
        std::function<void (void *)> const null_deleter = [](void *) {};

        char const * const errmsg[] = {
            "invalid cast from \'fusecpp_entry\' to \'file\'",
            "invalid cast from \'fusecpp_entry\' to \'directory\'"
        };

        generic_shared_null_ptr_t const shared_null_ptr;

    }   // namespace detail

    char const *get_errmsg(detail::errmsg_number num) noexcept
    {
        return detail::errmsg[num];
    }

}   // namespace planet

