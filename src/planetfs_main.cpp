#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#include <iostream>
#include <map>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <fusecpp.hpp>
#include <planet/common.hpp>
#include <planet/planet_handle.hpp>
#include <planet/planet_dns_op.hpp>
#include <planet/planet_tcp_client_op.hpp>
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

static int planet_mknod(char const *path, mode_t mode, dev_t device)
{
    syslog(LOG_INFO, "planet_mknod: creating %s mode=%o", path, mode);

    fusecpp::path_type p(path);
    if (fusecpp::search_file(root, path))
        return -EEXIST;
    auto dir_ptr = fusecpp::search_directory(root, p.parent_path());
    if (!dir_ptr)
        return -ENOENT;
    dir_ptr->create_file(p, mode);
    return 0;
}

static int planet_mkdir(const char *path, mode_t mode)
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

void do_planet_open(fusecpp::path_type const& path, struct fuse_file_info& fi)
{
    planet::planet_handle_t phandle;
    if (path == "/dns")
        phandle = planet::handle_mgr.register_op<planet::dns_op>();
    else if (path.parent_path() == "/eth/ip/tcp")
        phandle = planet::handle_mgr.register_op<planet::tcp_client_op>();
    else
        throw std::runtime_error(path.string() + " is not supported");
    planet::set_planet_handle_to(fi, phandle);
    planet::handle_mgr[phandle]->open(path, fi);
}

static int planet_open(char const *path, struct fuse_file_info *fi)
{
    try {
        if (!fusecpp::search_file(root, path))
            return -ENOENT;
        do_planet_open(path, *fi);
    } catch (std::exception& e) {
        syslog(LOG_INFO, "planet_open: %s: exception: %s", path, e.what());
        planet::handle_mgr.unregister_op(planet::get_planet_handle_from(*fi));
        return -ECONNRESET;
    }
    syslog(LOG_INFO, "planet_open: %s: opened handle=%d", path, planet::get_planet_handle_from(*fi));
    return 0;
}

static int planet_read(char const *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int bytes_received;
    try {
        auto phandle = planet::get_planet_handle_from(*fi);
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
        auto phandle = planet::get_planet_handle_from(*fi);
        syslog(LOG_INFO, "planet_write: writing %s size=%u, offset=%lld handle=%d", path, size, offset, phandle);
        bytes_transferred = planet::handle_mgr[phandle]->write(path, buf, size, offset, *fi);
    } catch (std::exception& e) {
        syslog(LOG_INFO, "planet_write: exception: %s", e.what());
        bytes_transferred = -EAGAIN;
    }
    return bytes_transferred;
}

static int planet_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    syslog(LOG_INFO, "planet_readdir: reading %s buf=%p, offset=%lld", path, buf, offset);

    auto dir = fusecpp::search_directory(root, path);
    if (!dir)
        return -ENOENT;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    for (auto ptr : dir->entries())
        if (filler(buf, ptr->path().filename().c_str(), NULL, 0))
            break;
    return 0;
}

static int planet_release(char const *path, struct fuse_file_info *fi)
{
    int ret;
    planet::planet_handle_t phandle = planet::get_planet_handle_from(*fi);
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
    // Create /net/eth/ip/tcp
    root.create_directory("/eth");
    if (auto dir_eth = fusecpp::search_directory(root, "/eth")) {
        dir_eth->create_directory("/eth/ip");
        if (auto dir_ip = fusecpp::search_directory(root, "/eth/ip"))
            dir_ip->create_directory("/eth/ip/tcp");
    }
    // Create /net/dns
    root.create_file("/dns", S_IFREG | S_IRWXU);
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
    syslog(LOG_INFO, "planetfs new handle version started");
    return fuse_main(argc, argv, &planet_ops, NULL);
}
