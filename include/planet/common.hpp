#ifndef PLANET_COMMON_HPP
#define PLANET_COMMON_HPP

#ifndef FUSE_USE_VERSION
    #define FUSE_USE_VERSION 26
#endif

#ifndef _FILE_OFFSET_BITS
    #define _FILE_OFFSET_BITS 64
#endif

#include <memory>
#include <functional>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <fuse/fuse.h>

// namespace for planetfs
namespace planet {

    // planet shared_ptr
    template<typename T>
    using shared_ptr = std::shared_ptr<T>;

    // planet string type
    typedef std::string string_type;
    
    // entry path type (better string class)
    typedef boost::filesystem::path path_type;

    template<typename T>
    using optional = boost::optional<T>;
    using boost::none;

    // file type number
    enum file_type {
        regular_file,
        directory
    };

    // planet operation number
    enum opcode {
        default_op,
        dns,
        tcp_client,
        tcp_server,
        tcp_accepted_client,
        packet_socket
    };

    namespace detail {

        extern std::function<void (void *)> const null_deleter;

        // Index numbers and error messages
        enum errmsg_number {fe_file_cast = 0, fe_dir_cast};
        extern char const * const errmsg[];

        // generic null pointer of shared_ptr
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

}   // namespace planet


#include <planet/exception.hpp>

#endif  // PLANET_COMMON_HPP
