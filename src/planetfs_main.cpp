#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#include <iostream>
#include <functional>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <fusecpp.hpp>
#include <planet/common.hpp>
#include <planet/planet_handle.hpp>
#include <planet/dns_op.hpp>
#include <planet/tcp_client_op.hpp>
#include <planet/tcp_server_op.hpp>
#include <planet/tcp_accepted_client_op.hpp>
#include <planet/packet_socket_op.hpp>
#include <fuse.h>

// Root of this filesystem
fusecpp::directory root{"/"};

static int planet_getattr(char const *path, struct stat *stbuf)
{
    int res = 0;

    std::memset(stbuf, 0, sizeof(struct stat));
    if (auto ptr = fusecpp::search_entry(root, path)) {
        syslog(LOG_INFO, "planet_getattr: path=%s,ptr_cnt=%ld", path, ptr.use_count());
        stbuf->st_nlink = ptr.use_count();
        if (ptr->is_directory()) {
            stbuf->st_mode = S_IFDIR | 0755;
        } else if (ptr->is_file()) {
            auto file_ptr = fusecpp::file_cast(ptr);
            stbuf->st_mode = file_ptr->get_mode();
            stbuf->st_size = file_ptr->data().size();
        }
    } else
        res = -ENOENT;
    return res;
}

int do_planet_mknod(fusecpp::path_type const& path, mode_t mode, dev_t, planet::opcode op,
        std::function<void (fusecpp::file&)> modifier = [](fusecpp::file&){})
{
    auto dir_ptr = fusecpp::search_directory(root, path.parent_path());
    if (!dir_ptr)
        return -ENOENT;
    dir_ptr->create_file(path, mode);
    if (auto ptr = fusecpp::search_file(root, path)) {
        ptr->private_data = op;
        modifier(*ptr);
    }
    return 0;
}

static int planet_mknod(char const *path, mode_t mode, dev_t device)
{
    syslog(LOG_INFO, "planet_mknod: creating %s mode=%o", path, mode);
    int ret = 0;
    try {
        if (fusecpp::search_file(root, path))
            return -EEXIST;
        planet::opcode op = planet::path_mgr.find_path_op(path);
        ret = do_planet_mknod(path, mode, device, op);
    } catch (planet::exception_errno& e) {
        ret = -e.get_errno();
    } catch (std::exception& e) {
        ret = -EIO;
    }
    return ret;
}

static int planet_mkdir(char const *path, mode_t mode)
{
    syslog(LOG_INFO, "planet_mkdir: creating %s mode=%o dirmode=%o", path, mode, mode | S_IFDIR);

    fusecpp::path_type p(path);
    if (fusecpp::search_directory(root, p))
        return -EEXIST;
    auto dir_ptr = fusecpp::search_directory(root, p.parent_path());
    if (!dir_ptr)
        return -ENOENT;
    dir_ptr->create_directory(p, mode | S_IFDIR);
    return 0;
}

void do_planet_open(fusecpp::file& file, struct fuse_file_info& fi)
{
    planet::planet_handle_t phandle;
    if (file.private_data == planet::opcode::dns)
        phandle = planet::handle_mgr.register_op<planet::dns_op>();
    else if (file.private_data == planet::opcode::tcp_client)
        phandle = planet::handle_mgr.register_op<planet::tcp_client_op>();
    else if (file.private_data == planet::opcode::tcp_server)
        phandle = planet::handle_mgr.register_op<planet::tcp_server_op>();
    else if (file.private_data == planet::opcode::tcp_accepted_client)
        phandle = planet::handle_mgr.register_op<planet::tcp_accepted_client_op>(file.path(), root);
    else if (file.private_data == planet::opcode::packet_socket)
        phandle = planet::handle_mgr.register_op<planet::packet_socket_op>();
    else
        throw std::runtime_error(file.path().string() + " is not supported");
    planet::set_handle_to(fi, phandle);
    planet::handle_mgr[phandle]->open(file.path(), fi);
}

static int planet_open(char const *path, struct fuse_file_info *fi)
{
    try {
        auto file_ptr = fusecpp::search_file(root, path);
        if (!file_ptr)
            return -ENOENT;
        do_planet_open(*file_ptr, *fi);
    } catch (planet::exception_errno& e) {
        syslog(LOG_INFO, "planet_open: %s: exception: %s", path, e.what());
        planet::handle_mgr.unregister_op(planet::get_handle_from(*fi));
        return -e.get_errno();
    } catch (std::exception& e) {
        syslog(LOG_INFO, "planet_open: %s: exception: %s", path, e.what());
        planet::handle_mgr.unregister_op(planet::get_handle_from(*fi));
        return -EIO;
    }
    syslog(LOG_INFO, "planet_open: %s: opened handle=%d", path, planet::get_handle_from(*fi));
    return 0;
}

static int planet_read(char const *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int bytes_received;
    try {
        auto phandle = planet::get_handle_from(*fi);
        syslog(LOG_INFO, "planet_read: reading %s size=%u, offset=%lld handle=%d", path, size, offset, phandle);
        bytes_received = planet::handle_mgr[phandle]->read(path, buf, size, offset, *fi);
        syslog(LOG_INFO, "planet_read: received %d bytes", bytes_received);
    } catch (std::exception& e) {
        syslog(LOG_INFO, "planet_read: exception: %s", e.what());
        bytes_received = -EIO;
    }
    return bytes_received;
}

static int planet_write(char const *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int bytes_transferred;
    try {
        auto phandle = planet::get_handle_from(*fi);
        syslog(LOG_INFO, "planet_write: writing %s size=%u, offset=%lld handle=%d", path, size, offset, phandle);
        bytes_transferred = planet::handle_mgr[phandle]->write(path, buf, size, offset, *fi);
    } catch (std::exception& e) {
        syslog(LOG_INFO, "planet_write: exception: %s", e.what());
        bytes_transferred = -EIO;
    }
    return bytes_transferred;
}

static int planet_readdir(char const *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    syslog(LOG_INFO, "planet_readdir: reading %s buf=%p, offset=%lld", path, buf, offset);

    auto dir = fusecpp::search_directory(root, path);
    if (!dir)
        return -ENOENT;
    filler(buf, ".", nullptr, 0);
    filler(buf, "..", nullptr, 0);
    for (auto ptr : dir->entries())
        if (filler(buf, ptr->path().filename().c_str(), nullptr, 0))
            break;
    return 0;
}

static int planet_release(char const *path, struct fuse_file_info *fi)
{
    int ret;
    planet::planet_handle_t phandle = planet::get_handle_from(*fi);
    try {
        syslog(LOG_INFO, "planet_release: %s closed handle=%d", path, phandle);
        ret = planet::handle_mgr[phandle]->release(path, *fi);
    } catch (...) {
        ret = -EIO;
    }
    planet::handle_mgr.unregister_op(phandle);
    return ret;
}

void planetfs_init()
{
    // Create initial directory structure
    root.create_directory("/eth");
    if (auto dir_eth = fusecpp::search_directory(root, "/eth")) {
        dir_eth->create_directory("/eth/ip");
        if (auto dir_ip = fusecpp::search_directory(root, "/eth/ip"))
            dir_ip->create_directory("/eth/ip/tcp");
    }
    do_planet_mknod("/dns", S_IFREG | S_IRWXU, 0, planet::opcode::dns);
    do_planet_mknod("/eth/eth0", S_IFREG | S_IRWXU, 0, planet::opcode::packet_socket);
    do_planet_mknod("/eth/wlan0", S_IFREG | S_IRWXU, 0, planet::opcode::packet_socket);

    // Register planet operations
    planet::path_mgr.register_op(planet::opcode::dns, planet::dns_op::is_matching_path);
    planet::path_mgr.register_op(planet::opcode::tcp_client, planet::tcp_client_op::is_matching_path);
    planet::path_mgr.register_op(planet::opcode::tcp_server, planet::tcp_server_op::is_matching_path);
    //planet::path_mgr.register_op(planet::opcode::tcp_accepted_client, planet::tcp_accepted_client_op::is_matching_path);
    planet::path_mgr.register_op(planet::opcode::packet_socket, planet::packet_socket_op::is_matching_path);
}

static struct fuse_operations planet_ops{};

int main(int argc, char **argv)
{
    planetfs_init();
    planet_ops.getattr  = planet_getattr;
    planet_ops.mknod    = planet_mknod;
    planet_ops.mkdir    = planet_mkdir;
    planet_ops.open     = planet_open;
    planet_ops.read     = planet_read;
    planet_ops.write    = planet_write;
    planet_ops.readdir  = planet_readdir;
    planet_ops.release  = planet_release;
    openlog("fuse_planet", LOG_CONS | LOG_PID, LOG_USER);
    return fuse_main(argc, argv, &planet_ops, nullptr);
}
