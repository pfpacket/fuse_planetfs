#ifndef PLANET_COMMON_HPP
#define PLANET_COMMON_HPP

#ifndef FUSE_USE_VERSION
#   define FUSE_USE_VERSION 26
#endif

#ifndef _FILE_OFFSET_BITS
#   define _FILE_OFFSET_BITS 64
#endif

#include <fuse.h>

#include <memory>
#include <functional>
#include <typeindex>
#include <type_traits>
#include <system_error>
#include <thread>
#include <atomic>
#include <mutex>

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/xpressive/xpressive.hpp>
// logging
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>

// namespace for planetfs
namespace planet {


    // planet shared_ptr
    template<typename T>
    using shared_ptr = std::shared_ptr<T>;

    template<typename T>
    using weak_ptr = std::weak_ptr<T>;

    using std::make_shared;

    // planet string type
    typedef std::string string_type;

    // entry path type (better string class)
    typedef boost::filesystem::path path_type;

    // FUSE poll mask type
    typedef unsigned pollmask_t;

    // Operation index code (deprecated)
    class op_type_code {
    private:
        string_type name_;
    public:
        op_type_code() = delete;

        explicit op_type_code(string_type const& type_name)
            :   name_(type_name)
        {
        }

        explicit op_type_code(std::type_info const& typeinfo)
            :   name_(typeinfo.name())
        {
        }

        template<typename OperationType>
        static op_type_code get(void)
        {
            return op_type_code(typeid(OperationType));
        }

        string_type const& name() const
        {
            return name_;
        }

        bool operator==(op_type_code const& r) const
        {
            return name_ == r.name_;
        }

        bool operator<(op_type_code const& r) const
        {
            return name_ < r.name_;
        }

        bool operator>(op_type_code const& r) const
        {
            return name_ > r.name_;
        }

        op_type_code& operator=(op_type_code const& r)
        {
            name_ = r.name_;
            return *this;
        }
    };

    template<typename T>
    using optional = boost::optional<T>;
    using boost::none;

    typedef boost::format format;
    using boost::str;

    using boost::lexical_cast;

    namespace xpv = boost::xpressive;

    // file type number
    enum file_type {
        regular_file,
        directory
    };

    namespace detail {

        // generic null pointer of shared_ptr
        class generic_shared_null_ptr_t {
        public:
            generic_shared_null_ptr_t()
            {
            }

            template<typename T>
            operator shared_ptr<T>() const
            {
                return shared_ptr<T>{};
            }
        };
        extern generic_shared_null_ptr_t const shared_null_ptr;

    }   // namespace detail

    [[noreturn]] extern void throw_system_error(int);

    [[noreturn]] extern void throw_system_error(int, std::string const&);


}   // namespace planet

#include <planet/exception.hpp>

#endif  // PLANET_COMMON_HPP
