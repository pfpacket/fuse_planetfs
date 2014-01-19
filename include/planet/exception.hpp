#ifndef PLANET_EXCEPTION_HPP
#define PLANET_EXCEPTION_HPP

#include <planet/common.hpp>
#include <typeinfo> // for std::bad_cast

namespace planet {


    class bad_file_cast : public std::bad_cast {
    private:
        const char *msg_ = "Invalid cast: not a file";

    public:
        bad_file_cast() : std::bad_cast()
        {
        }

        const char* what() const noexcept override
        {
            return msg_;
        }
    };

    class bad_dir_cast : public std::bad_cast {
    private:
        const char *msg_ = "Invalid cast: not a directory";

    public:
        bad_dir_cast() : std::bad_cast()
        {
        }

        const char* what() const noexcept override
        {
            return msg_;
        }
    };

    class no_such_ops_type : public std::runtime_error {
    private:
        string_type ops_name_;

    public:
        no_such_ops_type(string_type const& ops_name)
            :   std::runtime_error("No such operation type: " + ops_name),
                ops_name_(ops_name)
        {
        }

        no_such_ops_type(string_type const& msg, string_type const& ops_name)
            :   std::runtime_error(msg + ": " + ops_name), ops_name_(ops_name)
        {
        }

        virtual ~no_such_ops_type() noexcept
        {
        }

        string_type const& get_ops_name() const
        {
            return ops_name_;
        }
    };


}   // namespace planet

#endif  // PLANET_EXCEPTION_HPP
