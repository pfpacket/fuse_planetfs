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
#include <fuse.h>
#include <fusecpp.hpp>
#include "planet_socket.hpp"

// Root of this filesystem
fusecpp::directory root{"/"};

static int planet_getattr(char const *path, struct stat *stbuf)
{
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
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

static int planet_open(char const *path, struct fuse_file_info *fi)
{
    syslog(LOG_INFO, "planet_open: entered, opening %s", path);

    try {
        if (!fusecpp::search_file(root, path))
            return -ENOENT;
        fusecpp::path_type p(path);
        std::string filename = p.filename().string();
        auto pos = filename.find_first_of(':');
        std::string host = filename.substr(0, pos), port = filename.substr(pos + 1);

        fi->fh = open_planet_socket(path);
        auto psocket = get_planet_socket(fi);

        syslog(LOG_INFO, "planet_open: connecting to host=%s, port=%s fd=%d", host.c_str(), port.c_str(), psocket.sock);
        struct sockaddr_in sin = {AF_INET, htons(atoi(port.c_str())), {0}, {0}};
        inet_pton(AF_INET, host.c_str(), &sin.sin_addr);
        if (connect(psocket.sock, reinterpret_cast<struct sockaddr *>(&sin), sizeof(sin)) < 0)
            throw std::runtime_error(strerror(errno));
        syslog(LOG_NOTICE, "planet_open: connection established %s:%s fd=%d opened", host.c_str(), port.c_str(), psocket.sock);
    } catch (std::exception& e) {
        syslog(LOG_INFO, "planet_open: exception: %s", e.what());
        return -ECONNRESET;
    }
    return 0;
}

static int planet_read(char const *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    syslog(LOG_INFO, "planet_read: reading %s size=%u, offset=%lld", path, size, offset);
    int bytes_received;
    try {
        auto psocket = get_planet_socket(fi);
        bytes_received = ::recv(psocket.sock, buf, size, 0);
        if (bytes_received < 0)
            throw std::runtime_error(strerror(errno));
        syslog(LOG_INFO, "planet_read: received %d bytes", bytes_received);
    } catch (std::exception& e) {
        syslog(LOG_INFO, "planet_read: exception: %s", e.what());
        return -EIO;
    }
    return bytes_received;
}

static int planet_write(char const *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    syslog(LOG_INFO, "planet_write: entered");
    int bytes_transferred;
    try {
        auto psocket = get_planet_socket(fi);
        syslog(LOG_INFO, "planet_write: writing %s size=%u, offset=%lld fd=%d", path, size, offset, psocket.sock);
        bytes_transferred = ::send(psocket.sock, buf, size, 0);
        if (bytes_transferred < 0)
            throw std::runtime_error(strerror(errno));
    } catch (std::exception& e) {
        syslog(LOG_INFO, "planet_write: exception: %s", e.what());
        return -EAGAIN;
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
    auto psocket = get_planet_socket(fi);
    syslog(LOG_INFO, "planet_release: closing fd=%d", psocket.sock);
    ::close(get_planet_socket(fi).sock);
    handle_mapper.erase(get_planet_handle(fi));
    return 0;
}

void planetfs_init()
{
    root.create_directory("/eth");
    if (auto dir_eth = fusecpp::search_directory(root, "/eth")) {
        dir_eth->create_directory("/eth/ip");
        if (auto dir_ip = fusecpp::search_directory(root, "/eth/ip"))
            dir_ip->create_directory("/eth/ip/tcp");
    }
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
    return fuse_main(argc, argv, &planet_ops, NULL);
}
