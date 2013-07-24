
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
#include <planet/operation_layer.hpp>
#include <planet/net/tcp/clone_op.hpp>
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

    bool clone_op::target_ctl_is_connected(string_type const& ctl_path)
    {
        const char *request = "is_connected";
        auto handle = fs_root_.open(ctl_path);
        raii_wrapper raii([handle](){ planet::close(handle); });
        int ret = planet::write(handle, request, strlen(request), 0);
        return ret != -ENOTCONN;
    }

    shared_ptr<fs_operation> clone_op::new_instance()
    {
        auto next_ctl_path = str(format("/tcp/%1%/ctl") % current_fd_);
        // If current_fd_ ctl file isn't created, we have to create it first
        if (!fs_root_.get_entry_of(next_ctl_path))
            return std::make_shared<clone_op>(fs_root_, current_fd_);
        // Else, we confirm current ctl file is connected-state
        bool is_connected = target_ctl_is_connected(next_ctl_path);
        raii_wrapper finalizer([is_connected, this]()
                {
                    if (is_connected && std::uncaught_exception())
                        --current_fd_;
                });
        current_fd_ = is_connected ? current_fd_ + 1 : current_fd_;
        return std::make_shared<clone_op>(fs_root_, current_fd_);
    }

    int clone_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        auto session_dir_path = str(format("/tcp/%1%") % current_fd_);
        // Confirm the current fd session is started
        if (!fs_root_.get_entry_of(session_dir_path))
            fs_root_.mkdir(session_dir_path, S_IRWXU);
        ctl_handle_ = fs_root_.open(session_dir_path + "/ctl");
        return 0;
    }

    int clone_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        int bytes = planet::read(ctl_handle_, buf, size, offset);
        return bytes;
    }

    int clone_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        int bytes = planet::write(ctl_handle_, buf, size, offset);
        return bytes;
    }

    int clone_op::release(shared_ptr<fs_entry> file_ent)
    {
        int ret = planet::close(ctl_handle_);
        return ret;
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
