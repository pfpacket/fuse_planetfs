
#include <planet/tcp/common.hpp>

namespace planet {
namespace net {
namespace tcp {


    namespace path_reg {
        boost::regex ctl            {R"(/tcp/(\d+)/ctl)"};
        boost::regex data           {R"(/tcp/(\d+)/data)"};
        boost::regex local          {R"(/tcp/(\d+)/local)"};
        boost::regex remote         {R"(/tcp/(\d+)/remote)"};
        boost::regex session_dir    {R"(/tcp/(\d+))"};
        boost::regex dir_num        {R"(/tcp/(\d+)[/\w]*)"};
    }   // namespace path_reg

    namespace detail {
        optional<fd_table::mapped_type> fd_table::find(const fd_table::key_type& k) const
        {
            ::syslog(LOG_NOTICE, "fdtable.insert(): finding KEY=%s", k.c_str());
            auto it = table_.find(k);
            if (it == table_.end())
                return boost::none;
            else
                return it->second;
        }

        optional<fd_table::mapped_type> fd_table::find_from_path(const fd_table::key_type& path) const
        {
            return find(dir_number(path));
        }

        void fd_table::clear() noexcept
        {
            table_.clear();
        }

        bool fd_table::erase(fd_table::key_type const& k)
        {
            if (auto fd = find(k))
                ::close(*fd);
            return table_.erase(k);
        }

        bool fd_table::erase_from_path(fd_table::key_type const& path)
        {
            if (auto fd = find_from_path(path))
                ::close(*fd);
            return erase(dir_number(path));
        }

        fd_table::return_type fd_table::insert(fd_table::key_type const& k, fd_table::mapped_type fd)
        {
            ::syslog(LOG_NOTICE, "fdtable.insert(): registeting KEY=%s VALUE=%d", k.c_str(), fd);
            auto old_fd = this->find(k);
            if (old_fd)
                this->erase(k);
            return table_.insert(std::make_pair(k, fd));
        }

        fd_table::return_type fd_table::insert_from_path(fd_table::key_type const& path, fd_table::mapped_type fd)
        {
            return insert(dir_number(path), fd);
        }

        fd_table::key_type fd_table::dir_number(fd_table::key_type const& path)
        {
            boost::smatch m;
            boost::regex_match(path, m, path_reg::dir_num);
            syslog(LOG_NOTICE, "fdtable.dir_number(): path=%s,m[1]=%s", path.c_str(), m[1].str().c_str());
            return m[1];
        }

        fd_table fdtable;
    }   // namespace detail

    int sock_create(int domain, int type, int protocol)
    {
        int soc = ::socket(domain, type, protocol);
        if (soc < 0)
            throw exception_errno(errno);
        return soc;
    }

    int sock_create4()
    {
        return sock_create(sock_arg::domain, sock_arg::type, sock_arg::protocol);
    }

    int sock_create6()
    {
        return sock_create(sock_arg::domain6, sock_arg::type6, sock_arg::protocol6);
    }

    void sock_connect_to(int fd, std::string const& host, int port)
    {
        struct sockaddr_in sin = {AF_INET, htons(port), {0}, {0}};
        inet_pton(AF_INET, host.c_str(), &sin.sin_addr);
        if (connect(fd, reinterpret_cast<struct sockaddr *>(&sin), sizeof(sin)) < 0)
            throw planet::exception_errno(errno);
    }

    struct tcp_info sock_get_tcp_info(int sock)
    {
        struct tcp_info info;
        socklen_t info_length = sizeof (info);
        if (getsockopt(sock, SOL_TCP, TCP_INFO, &info, &info_length) != 0)
            throw exception_errno(errno);
        return info;
    }

    // getsockopt(TCP_INFO) depends on Linux functionality
    bool sock_is_connected(int sock)
    {
        return sock_get_tcp_info(sock).tcpi_state == TCP_ESTABLISHED;
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
