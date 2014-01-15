
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
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

    //
    // clone_op
    //
    const string_type clone_type::type_name = "planet.net.tcp.clone";

    int clone_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        auto session_dir_path = str(format("/tcp/%1%") % current_fd_);
        // Confirm the current fd session is started
        if (!fs_root_->get_entry_of(session_dir_path))
            fs_root_->mkdir(session_dir_path, S_IRWXU);
        ctl_handle_ = fs_root_->open(session_dir_path + "/ctl");
        return 0;
    }

    int clone_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        int bytes = fs_root_->read(ctl_handle_, buf, size, offset);
        return bytes;
    }

    int clone_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        int bytes = fs_root_->write(ctl_handle_, buf, size, offset);
        return bytes;
    }

    int clone_op::release(shared_ptr<fs_entry> file_ent)
    {
        int ret = fs_root_->close(ctl_handle_);
        return ret;
    }

    //
    // clone_type
    //
    bool clone_type::target_ctl_is_connected(string_type const& ctl_path,
            shared_ptr<core_file_system> fs_root)
    {
        const char *request = "is_connected";
        auto handle = fs_root->open(ctl_path);
        raii_wrapper raii([&fs_root,handle](){ fs_root->close(handle); });
        int ret = fs_root->write(handle, request, strlen(request), 0);
        return ret != -ENOTCONN;
    }

    shared_ptr<entry_op> clone_type::create_op(shared_ptr<core_file_system> fs_root)
    {
        auto next_ctl_path = str(format("/tcp/%1%/ctl") % current_fd_);
        // If current_fd_ ctl file isn't created, we have to create it first
        if (!fs_root->get_entry_of(next_ctl_path))
            return std::make_shared<clone_op>(fs_root, current_fd_);
        // Else, we confirm current ctl file is connected-state
        bool is_connected = target_ctl_is_connected(next_ctl_path, fs_root);
        raii_wrapper finalizer([is_connected, this]()
                {
                    if (is_connected && std::uncaught_exception())
                        --current_fd_;
                });
        current_fd_ = is_connected ? current_fd_ + 1 : current_fd_;
        return std::make_shared<clone_op>(fs_root, current_fd_);
    }

    int clone_type::mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t, dev_t)
    {
        return 0;
    }

    int clone_type::rmnod(shared_ptr<fs_entry> file_ent, path_type const&)
    {
        return -EPERM;
    }

    bool clone_type::match_path(path_type const& path, file_type type)
    {
        return type == file_type::regular_file && path == "/tcp/clone";
    }



}   // namespace tcp
}   // namespace net
}   // namespace planet
