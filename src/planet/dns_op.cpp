
#include <planet/common.hpp>
#include <planet/dns_op.hpp>

namespace planet {


int dns_op::forward_lookup(std::string const& hostname, int family, std::vector<std::string>& store)
{
    struct addrinfo hints = {}, *result;
    hints.ai_family     = family;       // AF_INET,AF_INET6,AF_UNSPEC
    hints.ai_socktype   = SOCK_STREAM;  // Stream socke

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

dns_op::~dns_op()
{
    syslog(LOG_INFO, "dns_op: dtor called target=%s", hostname_.c_str());
}

inline int dns_op::open(fusecpp::path_type const& path, struct fuse_file_info& fi)
{
    return 0;
}

int dns_op::read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi)
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

int dns_op::write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi)
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

inline int dns_op::release(fusecpp::path_type const& path, struct fuse_file_info& fi)
{
    return 0;
}

bool dns_op::is_matching_path(fusecpp::path_type const& path)
{
    return path == "/dns";
}


}   // namespace planet
