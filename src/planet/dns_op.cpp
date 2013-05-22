
#include <planet/common.hpp>
#include <planet/dns_op.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>

namespace planet {


    shared_ptr<planet_operation> dns_op::new_instance() const
    {
        return std::make_shared<dns_op>();
    }

    template<typename T, typename D>
    std::unique_ptr<T, D> make_unique_ptr(T *p, D d) noexcept
    {
        return std::unique_ptr<T, D>(p, std::forward<D>(d));
    }

    int dns_op::forward_lookup(std::string const& hostname, int family, std::vector<std::string>& store)
    {
        struct addrinfo hints = {}, *res;
        hints.ai_family     = family;       // AF_INET,AF_INET6,AF_UNSPEC
        hints.ai_socktype   = SOCK_STREAM;  // Stream socke

        int s = getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
        if (s != 0)
            return s;
        auto result = make_unique_ptr(res, [](struct addrinfo *ptr){ freeaddrinfo(ptr); });
        std::vector<char> buf(1024, 0);
        for (struct addrinfo *ai = result.get(); ai; ai = ai->ai_next) {
            if (ai->ai_family == AF_INET)
                inet_ntop(ai->ai_family, &((struct sockaddr_in *)ai->ai_addr)->sin_addr, buf.data(), buf.size());
            else if (ai->ai_family == AF_INET6)
                inet_ntop(ai->ai_family, &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr, buf.data(), buf.size());
            store.emplace_back(buf.data());
            std::fill(buf.begin(), buf.end(), 0);
        }
        return 0;
    }

    int dns_op::open(shared_ptr<file_entry> file_ent, path_type const& path)
    {
        return 0;
    }

    int dns_op::read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset)
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

    int dns_op::write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        std::string request(buf, size);
        auto pos = request.find_first_of(' ');
        if (pos == std::string::npos)
            return -EINVAL;
        std::string opecode = request.substr(0, pos), hostname_ = request.substr(pos + 1);
        syslog(LOG_INFO, "dns_op: write: opecode=%s / hostname=%s", opecode.c_str(), hostname_.c_str());

        int family;
        if (opecode == "resolve")            family = AF_UNSPEC;
        else if (opecode == "resolve_inet")  family = AF_INET;
        else if (opecode == "resolve_inet6") family = AF_INET6;
        else return -EINVAL;
        resolved_names_.clear();
        forward_lookup(hostname_, family, resolved_names_);
        return request.length();
    }

    int dns_op::release(shared_ptr<file_entry> file_ent)
    {
        return 0;
    }

    int dns_op::mknod(shared_ptr<file_entry>, path_type const&, mode_t, dev_t)
    {
        return 0;
    }

    int dns_op::rmnod(shared_ptr<file_entry>, path_type const&)
    {
        return 0;
    }

    bool dns_op::is_matching_path(path_type const& path)
    {
        return path == "/dns";
    }


}   // namespace planet
