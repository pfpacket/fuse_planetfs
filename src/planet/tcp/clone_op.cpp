
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
#include <planet/operation_layer.hpp>
#include <planet/tcp/clone_op.hpp>
#include <planet/utils.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>

namespace planet {
namespace net {
namespace tcp {

    //
    // Behavior example:
    //
    // $ cat /net/tcp/clone
    // 0
    // $ cat > /net/tcp/clone
    //        or
    // $ cat > /net/tcp/0/ctl
    // connect 127.0.0.1!7
    // $ echo "hello" > /net/tcp/0/data
    // $ cat /net/tcp/0/data
    // hello
    //

    bool target_ctl_is_connected(handle_t handle)
    {
        char buffer[1024];
        const char *request = "is_connected";
        planet::write(handle, request, strlen(request), 0);
        planet::read(handle, buffer, sizeof (buffer), 0);
        return std::string("true") == buffer;
    }

    shared_ptr<fs_operation> clone_op::new_instance()
    try {
        auto fmt = format("/tcp/%1%/ctl") % current_fd_;
        // If current_fd_ ctl file isn't created, we have to create it first
        if (!fs_root_.get_entry_of(str(fmt)))
            return std::make_shared<clone_op>(fs_root_, current_fd_);
        // Else, we confirm current ctl file is connected-state
        bool is_connected = target_ctl_is_connected(fs_root_.open(str(fmt)));
        auto new_op = std::make_shared<clone_op>(fs_root_, is_connected ? current_fd_ + 1 : current_fd_);
        if (is_connected)
            ++current_fd_;
        return new_op;
    } catch (...) {
        --current_fd_;
        throw;
    }

    int clone_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        auto fmt = format("/tcp/%1%") % current_fd_;
        if (!fs_root_.get_entry_of(path))
            fs_root_.mkdir(str(fmt), S_IRWXU);
        return 0;
    }

    int clone_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        std::string dir_number = str(format("%1%") % current_fd_);
        if (dir_number.length() >= size)
            return -ENOBUFS;
        std::copy(dir_number.begin(), dir_number.end(), buf);
        buf[dir_number.length()] = '\0';
        return 0;
    }

    int clone_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        auto handle = fs_root_.open(str(format("/net/tcp/%1%/ctl") % current_fd_));
        int bytes = planet::write(handle, buf, size, offset);
        planet::close(handle);
        return bytes;
    }

    int clone_op::release(shared_ptr<fs_entry> file_ent)
    {
        return 0;
    }

    int clone_op::mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t, dev_t)
    {
        return 0;
    }

    int clone_op::rmnod(shared_ptr<fs_entry> file_ent, path_type const&)
    {
        return -EPERM;
    }

    bool clone_op::is_matching_path(path_type const& path, file_type type)
    {
        return type == file_type::regular_file && path == "/tcp/clone";
    }


}   // namespace tcp
}   // namespace net
}   // namespace planet
