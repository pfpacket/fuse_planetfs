#ifndef FUSECPP_COMMON_HPP
#define FUSECPP_COMMON_HPP

#if defined(FUSECPP_BAD_CAST_THROW_EXCEPTION)
#endif

#if defined(FUSECPP_BAD_CAST_THROW_EXCEPTION_IN_SEARCH)
#endif

#include <memory>
#include <functional>
#include <boost/filesystem/path.hpp>

namespace fusecpp {

    // fusecpp shared_ptr
    template<typename T>
    using shared_ptr = std::shared_ptr<T>;

    // fusecpp entry path type (better string class)
    typedef boost::filesystem::path path_type;

    namespace detail {

        extern std::function<void (void *)> const null_deleter;

        // fusecpp index numbers and error messages
        enum errmsg_number {fe_file_cast = 0, fe_dir_cast};
        extern char const * const errmsg[];

        // Generic null pointer of shared_ptr
        class generic_shared_null_ptr_t {
        public:
            template<typename T>
            operator shared_ptr<T>()
            {
                return shared_ptr<T>{};
            }
        };
        extern generic_shared_null_ptr_t shared_null_ptr;

    }   // namespace detail

    char const *get_errmsg(detail::errmsg_number num) noexcept;

}   // namespace fusecpp


#endif  // FUSECPP_COMMON_HPP
