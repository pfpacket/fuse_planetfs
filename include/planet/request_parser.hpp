#ifndef PLANET_REQUEST_PARSER_HPP
#define PLANET_REQUEST_PARSER_HPP

#include <tuple>
#include <boost/optional.hpp>
#include <boost/xpressive/xpressive.hpp>

namespace planet {


    class request_parser {
    public:
        typedef std::string string_type;
        typedef std::vector<string_type> match_type;
        typedef std::vector<match_type> args_type;
        enum result_index { command = 0, args };

        request_parser();

        bool parse(string_type const& str);

        string_type get_command() const;

        args_type get_args(
            string_type const& arg_delim = ",",
            boost::xpressive::sregex const& arg_reg
                = boost::xpressive::sregex::compile(".*")
        ) const;

        args_type get_filtered_args(
            string_type const& arg_reg_str,
            string_type const& arg_delim = ","
        ) const;

        template<typename RegT>
        args_type get_filtered_args(
            RegT const& arg_reg,
            string_type const& arg_delim = ",",
            typename std::enable_if<std::is_same<RegT, boost::xpressive::sregex>::value>::type* = 0
        ) const
        {
            return this->get_args(arg_delim, arg_reg);
        }

    private:
        boost::optional<std::tuple<string_type, string_type>> result_ = boost::none;

        static boost::xpressive::mark_tag const command_, args_;
        static boost::xpressive::sregex const word_, delim_, r_args_, request_;
    };


}   //planet

#endif  // PLANET_REQUEST_PARSER_HPP
