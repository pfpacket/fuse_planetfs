
#include <planet/net/common.hpp>

namespace planet {
namespace net {


    namespace path_reg {
        xpv::sregex dir_num = xpv::sregex::compile(R"(/\w+/(\d+)[/\w]*)");
    }   // namespace path_reg

    namespace detail {
        optional<fd_table::mapped_type> fd_table::find(const fd_table::key_type& k) const
        {
            ::syslog(LOG_NOTICE, "fdtable.find(): finding KEY=%s", k.c_str());
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
            ::syslog(LOG_NOTICE, "fdtable.erase(): unregisteting KEY=%s", k.c_str());
            return table_.erase(k);
        }

        bool fd_table::erase_from_path(fd_table::key_type const& path)
        {
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
            xpv::smatch m;
            if (!xpv::regex_match(path, m, path_reg::dir_num) || m.size() < 2)
                throw std::runtime_error("fd_table::dir_number(): failed to parse directory number: " + path);
            syslog(LOG_NOTICE, "fdtable.dir_number(): path=%s,m[1]=%s", path.c_str(), m[1].str().c_str());
            return m[1];
        }

        fd_table fdtable;
    }   // namespace detail

    void get_name_info(sockaddr const *peer, int addrlen, string_type& node, string_type& serv, int flags)
    {
        char hostname[NI_MAXHOST], servname[NI_MAXSERV];
        int r = getnameinfo(
            peer, addrlen,
            hostname, sizeof (hostname), servname, sizeof (servname), flags
        );
        if (r != 0)
            throw std::runtime_error(str(format("getnameinfo: %1%") % gai_strerror(r)));
        node = hostname;
        serv = servname;
    }



}   // namespace net
}   // namespace planet
