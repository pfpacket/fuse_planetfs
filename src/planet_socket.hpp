#ifndef PLANET_SOCKET_HPP
#define PLANET_SOCKET_HPP

#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

template<typename T>
using shared_ptr = std::shared_ptr<T>;


//
// planet core handler
//
class planet_operations {
public:
    virtual int open(fusecpp::path_type const& path, struct fuse_file_info& fi) = 0;
    virtual int read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi) = 0;
    virtual int write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi) = 0;
    virtual int release(fusecpp::path_type const& path, struct fuse_file_info& fi) = 0;
};

class planet_tcp_client_ops : public planet_operations {
    int port_, fd_;
    std::string host_;
    std::vector<std::string> resolved_names_;
public:
    virtual ~planet_tcp_client_ops()
    {
        syslog(LOG_INFO, "planet_tcp_client_ops: dtor called fd=%d", fd_);
    }

    int open(fusecpp::path_type const& path, struct fuse_file_info& fi)
    {
        fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        std::string filename = path.filename().string();
        auto pos = filename.find_first_of(':');
        host_ = filename.substr(0, pos), port_ = atoi(filename.substr(pos + 1).c_str());
        syslog(LOG_INFO, "planet_open: connecting to host=%s, port=%d fd=%d", host_.c_str(), port_, fd_);

        struct sockaddr_in sin = {AF_INET, htons(port_), {0}, {0}};
        inet_pton(AF_INET, host_.c_str(), &sin.sin_addr);
        if (connect(fd_, reinterpret_cast<struct sockaddr *>(&sin), sizeof(sin)) < 0)
            throw std::runtime_error(strerror(errno));
        syslog(LOG_NOTICE, "planet_open: connection established %s:%d fd=%d opened", host_.c_str(), port_, fd_);
        return fd_;
    }

    int read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi)
    {
        return ::recv(fd_, buf, size, 0);
    }

    int write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi)
    {
        return ::send(fd_, buf, size, 0);
    }

    int release(fusecpp::path_type const& path, struct fuse_file_info& fi)
    {
        return ::close(fd_);
    }
};

class planet_dns_ops : public planet_operations {
    std::string hostname_;
    std::vector<std::string> resolved_names_;

    int resolve(std::string const& hostname, int family)
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
            resolved_names_.emplace_back(buf.data());
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
        resolve(hostname_, family);
        return hostname_.length();
    }

    int release(fusecpp::path_type const& path, struct fuse_file_info& fi)
    {
        return 0;
    }
};


// planet handle type
typedef int planet_handle_t;

class planet_handle_manager {
private:
    std::mutex mtx_;
    typedef std::lock_guard<decltype(mtx_)> lock_guard;
    planet_handle_t current_;
    std::map<planet_handle_t, shared_ptr<planet_operations>> ops_;
public:
    planet_handle_manager(int init = 0) : current_(init)
    {
    }

    shared_ptr<planet_operations> operator[](planet_handle_t index)
    {
        return ops_.at(index);
    }

    template<typename T>
    planet_handle_t register_op()
    {
        lock_guard lock(mtx_);
        ops_.insert(std::make_pair(current_ + 1, std::make_shared<T>()));
        return ++current_;
    }

    void unregister_op(planet_handle_t ph)
    {
        lock_guard lock(mtx_);
        ops_.erase(ph);
    }
};

planet_handle_manager handle_mgr;

planet_handle_t get_planet_handle_from(struct fuse_file_info const& fi)
{
    return static_cast<planet_handle_t>(fi.fh);
}

void set_planet_handle_to(struct fuse_file_info& fi, planet_handle_t ph)
{
    fi.fh = static_cast<decltype(fi.fh)>(ph);
}


#endif  // PLANET_SOCKET_HPP
