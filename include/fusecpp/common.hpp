#ifndef FUSECPP_COMMON_HPP
#define FUSECPP_COMMON_HPP

#if defined(FUSECPP_BAD_CAST_THROW_EXCEPTION)
#endif

#if defined(FUSECPP_BAD_CAST_THROW_EXCEPTION_IN_SEARCH)
#endif

#include <memory>

namespace fusecpp {

    // fusecpp shared_ptr
    template<typename T>
    using shared_ptr = std::shared_ptr<T>;

    namespace detail {

        static auto const null_deleter = [](void *) {};

        // fusecpp index numbers and error messages
        enum errmsg_number {fe_file_cast = 0, fe_dir_cast};
        static char const * const errmsg[] = {
            "invalid cast from \'fusecpp_entry\' to \'file\'",
            "invalid cast from \'fusecpp_entry\' to \'directory\'"
        };

        // Generic null pointer of shared_ptr
        static class {
        public:
            template<typename T>
            operator shared_ptr<T>()
            {
                return shared_ptr<T>{};
            }
        } shared_null_ptr;

    }   // namespace detail

    char const *get_errmsg(detail::errmsg_number num) noexcept
    {
        return detail::errmsg[num];
    }

}   // namespace fusecpp


#endif  // FUSECPP_COMMON_HPP
