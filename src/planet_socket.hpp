#ifndef PLANET_SOCKET_HPP
#define PLANET_SOCKET_HPP

typedef int planet_handle_t;

planet_handle_t next_handle = 0;


struct planet_socket_t {
    int domain, type, protocol, socket;
    planet_socket_t(int d, int t, int p, int s)
        :   domain(d), type(t), protocol(p), socket(s)
    {
        if (s < 0)
            throw std::runtime_error("planet_socket_t ctor: invalid socket");
    }
};


std::map<planet_handle_t, planet_socket_t> handle_mapper;


planet_handle_t open_planet_socket(fusecpp::path_type const& path)
{
    if (path.parent_path() != "/eth/ip/tcp")
        throw std::runtime_error("invalid argument");
    handle_mapper.insert(
        std::make_pair(next_handle + 1,
            planet_socket_t{AF_INET, SOCK_STREAM, 0, socket(AF_INET, SOCK_STREAM, 0)})
    );
    return ++next_handle;
}


planet_handle_t get_planet_handle(struct fuse_file_info *fi) noexcept
{
    return static_cast<planet_handle_t>(fi->fh);
}


planet_socket_t& get_planet_socket(struct fuse_file_info *fi)
{
    return handle_mapper.at(get_planet_handle(fi));
}


#endif  // PLANET_SOCKET_HPP
