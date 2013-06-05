
#include <planet/common.hpp>

namespace planet {

    namespace detail {

        std::function<void (void *)> const null_deleter = [](void *) {};

        char const * const errmsg[] = {
            "invalid cast from \'fs_entry\' to \'file_entry\'",
            "invalid cast from \'fs_entry\' to \'dentry\'"
        };

        generic_shared_null_ptr_t const shared_null_ptr;

    }   // namespace detail

    char const *get_errmsg(detail::errmsg_number num) noexcept
    {
        return num >= detail::pe_end ? nullptr : detail::errmsg[num];
    }

}   // namespace planet

