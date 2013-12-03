#ifndef PLANET_NET_COMMON_HPP
#define PLANET_NET_COMMON_HPP

#include <planet/common.hpp>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <planet/fs_ops_type.hpp>

namespace planet {
namespace net {


    namespace path_reg {
        extern xpv::sregex dir_num;
    }   // namespace path_reg

    namespace detail {
        // Session socket descriptor set
        class fd_table {
        public:
            typedef string_type key_type;
            typedef int mapped_type;
            typedef std::map<key_type, mapped_type> map_type;
            typedef map_type::iterator iterator;
            typedef map_type::const_iterator const_iterator;
            template<typename T1, typename T2>
            using pair = std::pair<T1, T2>;
            typedef pair<iterator, bool> return_type;

            fd_table() = default;
            optional<mapped_type> find(const key_type& k) const;
            optional<mapped_type> find_from_path(const key_type& path) const;
            void clear() noexcept;
            bool erase(key_type const& k);
            bool erase_from_path(key_type const& path);
            return_type insert(key_type const& k, mapped_type fd);
            return_type insert_from_path(key_type const& path, mapped_type fd);
        private:
            static key_type dir_number(key_type const& path);
            map_type table_;
        };
        extern fd_table fdtable;
    }   // namespace detail

    extern void get_name_info(sockaddr const *peer, int addrlen, string_type& node, string_type& serv, int flags = 0);


}   // namespace net
}   // namespace planet

#endif  // PLANET_NET_COMMON_HPP
