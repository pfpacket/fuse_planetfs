
#include <planet/request_parser.hpp>

namespace planet {


    namespace xpv = boost::xpressive;

    xpv::mark_tag const
        request_parser::command_{1},
        request_parser::args_{2};
    xpv::sregex const
        request_parser::word_   = +(xpv::_w | xpv::_d | '.'),
        request_parser::delim_  = +(xpv::as_xpr(' ') | xpv::as_xpr('\t')),
        request_parser::r_args_ = +(~xpv::_n);
    xpv::sregex const
        request_parser::request_ =
            ( ((command_ = word_) >> delim_ >> (args_ = r_args_))
                | (command_ = word_) ) >> *xpv::as_xpr('\n');

    request_parser::request_parser()
    {
    }

    bool request_parser::parse(
        string_type const& str
    )
    {
        xpv::smatch what;
        if (!xpv::regex_match(str, what, request_))
            return false;
        result_ = std::make_tuple(what[command_], what[args_]);
        return true;
    }

    request_parser::string_type request_parser::get_command() const
    {
        if (!result_)
            throw std::runtime_error("request_parser: empty result");
        return std::get<0>(*result_);
    }

    request_parser::args_type request_parser::get_args(
        string_type const& arg_delim, xpv::sregex const& arg_reg
    ) const
    {
        if (!result_)
            throw std::runtime_error("request_parser: empty result");
        args_type args_store;
        auto&& args_str = std::get<1>(*result_);
        if (args_str.length()) {
            typedef xpv::sregex_token_iterator it_type;
            it_type token_end,
                token_begin(args_str.begin(), args_str.end(), xpv::as_xpr(arg_delim), -1);
            std::for_each(token_begin, token_end,
                [&arg_reg, &args_store](it_type::value_type const& v){
                    xpv::smatch m;
                    match_type matches;
                    if (xpv::regex_match(v.str(), m, arg_reg)) {
                        for (auto&& match : m)
                            matches.push_back(match);
                        args_store.push_back(matches);
                    }
                });
        }
        return args_store;
    }

    request_parser::args_type request_parser::get_filtered_args(
        string_type const& arg_reg_str,
        string_type const& arg_delim
    ) const
    {
        return this->get_args(arg_delim, xpv::sregex::compile(arg_reg_str));
    }


}   // namespace planet
