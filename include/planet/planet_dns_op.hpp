#ifndef PLANET_DNS_OP_HPP
#define PLANET_DNS_OP_HPP

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <planet/planet_handle.hpp>


class planet_dns_ops : public planet_operations {
    std::string hostname_;
    std::vector<std::string> resolved_names_;

    static int forward_lookup(std::string const& hostname, int family, std::vector<std::string>& store);
public:
    virtual ~planet_dns_ops();
    int open(fusecpp::path_type const& path, struct fuse_file_info& fi);
    int read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi);
    int write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi);
    int release(fusecpp::path_type const& path, struct fuse_file_info& fi);
};


#endif  // PLANET_DNS_OP_HPP
