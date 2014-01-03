
#include <planet/common.hpp>
#include <netdb.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <planet/utils.hpp>
#include <planet/net/dns/resolver_op.hpp>
#include <planet/request_parser.hpp>

namespace planet {
namespace net {
namespace dns {


    int resolver_op::forward_lookup(std::string const& host, int family, std::vector<std::string>& store)
    {
        struct addrinfo hints{}, *res;
        hints.ai_family     = family;       // AF_INET,AF_INET6,AF_UNSPEC
        hints.ai_socktype   = SOCK_STREAM;  // Stream socket

        int s = getaddrinfo(host.c_str(), nullptr, &hints, &res);
        if (s != 0)
            throw std::runtime_error(gai_strerror(s));
        auto result = make_unique_ptr(res, [](struct addrinfo *ptr){ freeaddrinfo(ptr); });
        for (struct addrinfo *ai = result.get(); ai; ai = ai->ai_next) {
            std::string hostname, servname;
            get_name_info(ai->ai_addr, ai->ai_addrlen,
                    hostname, servname, NI_NUMERICHOST | NI_NUMERICSERV);
            store.emplace_back(hostname);
        }
        return 0;
    }

    int resolver_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        return 0;
    }

    int resolver_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        if (resolved_names_.empty())
            return 0;
        std::string line = resolved_names_.front();
        if (line.length() + 1 > size)
            return -ENAMETOOLONG;
        std::copy_n(line.begin(), line.length(), buf);
        buf[line.length()] = '\n';
        resolved_names_.erase(resolved_names_.begin());
        return line.length() + 1;
    }

    int resolver_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        request_parser parser;
        if (!parser.parse(string_type(buf, size)))
            return -ENOTSUP;
        static std::map<string_type, int> const families
            = {{"resolve", AF_UNSPEC}, {"resolve_inet", AF_INET}, {"resolve_inet6", AF_INET6}};
        auto&& args = parser.get_args();
        if (args.empty() || args.size() >= 2)
            return -ENOTSUP;
        syslog_fmt(LOG_INFO, format("resolver_op: command=%s / hostname=%s") % parser.get_command() % args[0][0]);
        resolved_names_.clear();
        forward_lookup(args[0][0], families.at(parser.get_command()), resolved_names_);
        return size;
    }

    int resolver_op::release(shared_ptr<fs_entry> file_ent)
    {
        return 0;
    }


}   // namespace dns
}   // namespace net
}   // namespace planet
