
#include <planet/common.hpp>
#include <planet/dns/resolver_op.hpp>
#include <planet/utils.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>

namespace planet {
namespace net {
namespace dns {


//    resolver_op::resolver_op(core_file_system& fs_root)
//    {
//        if (!fs_root.get_entry_of("/dns/clone")) {
//            fs_root.install_op<dns_clone_op>(fs_root);
//            fs_root.mknod("/dns/clone", S_IWUSR | S_IRUSR, 0, );
//        }
//    }

    shared_ptr<fs_operation> resolver_op::new_instance()
    {
        return std::make_shared<resolver_op>();
    }

    int resolver_op::forward_lookup(std::string const& hostname, int family, std::vector<std::string>& store)
    {
        struct addrinfo hints = {}, *res;
        hints.ai_family     = family;       // AF_INET,AF_INET6,AF_UNSPEC
        hints.ai_socktype   = SOCK_STREAM;  // Stream socket

        int s = getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
        if (s != 0)
            throw std::runtime_error(gai_strerror(s));
        auto result = make_unique_ptr(res, [](struct addrinfo *ptr){ freeaddrinfo(ptr); });
        std::vector<char> buf(1024, 0);
        for (struct addrinfo *ai = result.get(); ai; ai = ai->ai_next) {
            if (ai->ai_family == AF_INET)
                inet_ntop(ai->ai_family, &((struct sockaddr_in *)ai->ai_addr)->sin_addr, buf.data(), buf.size());
            else if (ai->ai_family == AF_INET6)
                inet_ntop(ai->ai_family, &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr, buf.data(), buf.size());
            store.emplace_back(buf.data());
            std::fill(buf.begin(), buf.end(), '\0');
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
        std::string const& resolved = resolved_names_.front();
        std::size_t length = resolved.length();
        if (size <= length)
            return -ENOBUFS;
        std::copy(resolved.begin(), resolved.end(), buf);
        buf[length] = '\0';
        resolved_names_.erase(resolved_names_.begin());
        return length + 1;
    }

    int resolver_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        std::string request(buf, size);
        auto pos = request.find_first_of(' ');
        if (pos == std::string::npos)
            return -EINVAL;
        std::string opecode = request.substr(0, pos), hostname_ = request.substr(pos + 1);
        syslog(LOG_INFO, "resolver_op: write: opecode=%s / hostname=%s", opecode.c_str(), hostname_.c_str());

        int family;
        if (opecode == "resolve")            family = AF_UNSPEC;
        else if (opecode == "resolve_inet")  family = AF_INET;
        else if (opecode == "resolve_inet6") family = AF_INET6;
        else return -EINVAL;
        resolved_names_.clear();
        forward_lookup(hostname_, family, resolved_names_);
        return request.length();
    }

    int resolver_op::release(shared_ptr<fs_entry> file_ent)
    {
        return 0;
    }

    int resolver_op::mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t)
    {
        return 0;
    }

    int resolver_op::rmnod(shared_ptr<fs_entry>, path_type const&)
    {
        return -EPERM;
    }

    bool resolver_op::is_matching_path(path_type const& path, file_type type)
    {
        return type == file_type::regular_file && path == "/dns";
    }


}   // namespace dns
}   // namespace net
}   // namespace planet
