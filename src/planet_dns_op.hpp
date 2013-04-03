#ifndef PLANET_DNS_OP_HPP
#define PLANET_DNS_OP_HPP

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "planet_handle.hpp"


class planet_dns_ops : public planet_operations {
    std::string hostname_;
    std::vector<std::string> resolved_names_;

    static int forward_lookup(std::string const& hostname, int family, std::vector<std::string>& store)
    {
        struct addrinfo hints = {}, *result;
        hints.ai_family = family;           // AF_INET,AF_INET6,AF_UNSPEC
        hints.ai_socktype = SOCK_STREAM;    // Stream socke

        int s = getaddrinfo(hostname.c_str(), NULL, &hints, &result);
        if (s != 0)
            return s;
        std::vector<char> buf(1024, 0);
        for (struct addrinfo *ai = result; ai; ai = ai->ai_next) {
            if (ai->ai_family == AF_INET)
                inet_ntop(ai->ai_family, &((struct sockaddr_in *)ai->ai_addr)->sin_addr, buf.data(), buf.size());
            else if (ai->ai_family == AF_INET6)
                inet_ntop(ai->ai_family, &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr, buf.data(), buf.size());
            store.emplace_back(buf.data());
            std::fill(buf.begin(), buf.end(), 0);
        }
        freeaddrinfo(result);
        return 0;
    }

public:
    virtual ~planet_dns_ops()
    {
        syslog(LOG_INFO, "planet_dns_ops: dtor called target=%s", hostname_.c_str());
    }

    int open(fusecpp::path_type const& path, struct fuse_file_info& fi)
    {
        return 0;
    }
    
    int read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi)
    {
        if (resolved_names_.empty())
            return 0;
        std::string& resolved = *resolved_names_.begin();
        std::size_t length = resolved.length();
        if (size <= length)
            return -ENOBUFS;
        std::copy(resolved.begin(), resolved.end(), buf);
        buf[length] = '\0';
        resolved_names_.erase(resolved_names_.begin());
        return length + 1;
    }

    int write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi)
    {
        std::string request = buf;
        auto pos = request.find_first_of(' ');
        if (pos == std::string::npos)
            return -EINVAL;
        std::string opecode = request.substr(0, pos), hostname_ = request.substr(pos + 1);
        syslog(LOG_INFO, "planet_dns_ops: write: opecode=%s / hostname=%s", opecode.c_str(), hostname_.c_str());

        int family;
        if (opecode == "resolve")            family = AF_UNSPEC;
        else if (opecode == "resolve_inet")  family = AF_INET;
        else if (opecode == "resolve_inet6") family = AF_INET6;
        else return -EINVAL;
        resolved_names_.clear();
        forward_lookup(hostname_, family, resolved_names_);
        return hostname_.length();
    }

    int release(fusecpp::path_type const& path, struct fuse_file_info& fi)
    {
        return 0;
    }
};


#endif  // PLANET_DNS_OP_HPP
