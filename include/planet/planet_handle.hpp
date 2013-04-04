#ifndef PLANET_SOCKET_HPP
#define PLANET_SOCKET_HPP

#include <map>
#include <mutex>
#include <memory>
#include <utility>
#include <cerrno>
#include <planet/common.hpp>
#include <fusecpp/common.hpp>

namespace planet {

template<typename T>
using shared_ptr = std::shared_ptr<T>;


//
// planet core handler
//
class planet_operation {
public:
    virtual int open(fusecpp::path_type const& path, struct fuse_file_info& fi) = 0;
    virtual int read(fusecpp::path_type const& path, char *buf, size_t size, off_t offset, struct fuse_file_info& fi) = 0;
    virtual int write(fusecpp::path_type const& path, char const *buf, size_t size, off_t offset, struct fuse_file_info& fi) = 0;
    virtual int release(fusecpp::path_type const& path, struct fuse_file_info& fi) = 0;
};


// planet handle type
typedef int planet_handle_t;

class planet_handle_manager {
private:
    std::mutex mtx_;
    typedef std::lock_guard<decltype(mtx_)> lock_guard;
    planet_handle_t current_;
    std::map<planet_handle_t, shared_ptr<planet_operation>> ops_;

public:
    planet_handle_manager(int init = 0);

    shared_ptr<planet_operation> operator[](planet_handle_t index);

    template<typename OpsType, typename... Types>
    planet_handle_t register_op(Types&&... args)
    {
        lock_guard lock(mtx_);
        ops_.insert(
            std::make_pair(
                current_ + 1,
                std::make_shared<OpsType>(std::forward<Types>(args)...)
            )
        );
        return ++current_;
    }

    void unregister_op(planet_handle_t ph);
};

extern planet_handle_manager handle_mgr;

planet_handle_t get_handle_from(struct fuse_file_info const& fi);

void set_handle_to(struct fuse_file_info& fi, planet_handle_t ph);

}   // namespace planet

#endif  // PLANET_SOCKET_HPP
