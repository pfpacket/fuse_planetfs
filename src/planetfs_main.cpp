#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <iostream>
#include <memory>
#include <functional>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/operation_layer.hpp>
#include <planet/planet_handle.hpp>
#include <planet/dns_op.hpp>
#include <planet/tcp_client_op.hpp>
#include <planet/tcp_server_op.hpp>
#include <planet/packet_socket_op.hpp>
#include <syslog.h>

#define LOG_EXCEPTION_MSG(e) \
    ::syslog(LOG_ERR, "%s: %s", __func__, (e).what());

// Core filesystem object
planet::core_file_system fs_root(S_IRWXU);

static int planet_getattr(char const *path, struct stat *stbuf)
{
    ::syslog(LOG_INFO, "%s: path=%s stbuf=%p", __func__, path, stbuf);
    int ret = 0;
    try {
        std::memset(stbuf, 0, sizeof (struct stat));
        fs_root.getattr(path, *stbuf);
        ::syslog(LOG_INFO, "getattr: path=%s size=%llu mode=%o nlink=%d",
            path, stbuf->st_size, stbuf->st_mode, stbuf->st_nlink);
    } catch (planet::exception_errno& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.get_errno();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

static int planet_mknod(char const *path, mode_t mode, dev_t device)
{
    ::syslog(LOG_INFO, "%s: path=%s mode=%o %lld", __func__, path, mode, device);
    int ret = 0;
    try {
        ret = fs_root.mknod(path, mode, device);
    } catch (planet::exception_errno& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.get_errno();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

static int planet_unlink(char const *path)
{
    ::syslog(LOG_INFO, "%s: path=%s", __func__, path);
    int ret = 0;
    try {
        ret = fs_root.unlink(path);
    } catch (planet::exception_errno& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.get_errno();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

static int planet_mkdir(char const *path, mode_t mode)
{
    ::syslog(LOG_INFO, "%s: path=%s mode=%o", __func__, path, mode);
    int ret = 0;
    try {
        ret = fs_root.mkdir(path, 0755);
    } catch (planet::exception_errno& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.get_errno();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

static int planet_chmod(char const *path, mode_t mode)
{
    return 0;
}

static int planet_chown(char const *path, uid_t uid, gid_t gid)
{
    return 0;
}

static int planet_truncate(char const *path, off_t offset)
{
    return 0;
}

static int planet_utimens(char const* path, struct timespec const tv[2])
{
    int ret = 0;
    try {
        namespace ch = std::chrono;
        if (auto entry = fs_root.get_entry_of(path)) {
            ch::nanoseconds nano_access(tv[0].tv_nsec), nano_mod(tv[1].tv_nsec);
            ch::seconds sec_access(tv[0].tv_sec), sec_mod(tv[1].tv_sec);
            planet::st_inode new_inode = entry->inode();
            new_inode.atime = decltype(new_inode.atime)
                (ch::duration_cast<decltype(new_inode.atime)::duration>(nano_access + sec_access));
            new_inode.mtime = decltype(new_inode.atime)
                (ch::duration_cast<decltype(new_inode.atime)::duration>(nano_mod + sec_mod));
            entry->inode(new_inode);
        } else
            throw planet::exception_errno(ENOENT);
    } catch (planet::exception_errno& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.get_errno();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

static int planet_open(char const *path, struct fuse_file_info *fi)
{
    ::syslog(LOG_INFO, "%s: path=%s fi=%p", __func__, path, fi);
    int ret = 0;
    try {
        planet::handle_t ph = fs_root.open(path);
        planet::set_handle_to(*fi, ph);
        ::syslog(LOG_INFO, "%s: handle=%d", __func__, ph);
    } catch (planet::exception_errno& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.get_errno();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

static int planet_read(char const *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    ::syslog(LOG_INFO, "%s: path=%s buf=%p size=%d offset=%llu", __func__, path, buf, size, offset);
    int bytes_received;
    try {
        planet::handle_t ph = planet::get_handle_from(*fi);
        bytes_received = planet::read(ph, buf, size, offset);
        ::syslog(LOG_INFO, "%s: handle=%d bytes_received=%d", __func__, ph, bytes_received);
    } catch (planet::exception_errno& e) {
        LOG_EXCEPTION_MSG(e);
        bytes_received = -e.get_errno();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        bytes_received = -EIO;
    }
    return bytes_received;
}

static int planet_write(char const *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    ::syslog(LOG_INFO, "%s: path=%s buf=%p size=%d offset=%llu", __func__, path, buf, size, offset);
    int bytes_transferred;
    try {
        planet::handle_t ph = planet::get_handle_from(*fi);
        bytes_transferred = planet::write(ph, buf, size, offset);
        ::syslog(LOG_INFO, "%s: handle=%d bytes_transferred=%d", __func__, ph, bytes_transferred);
    } catch (planet::exception_errno& e) {
        LOG_EXCEPTION_MSG(e);
        bytes_transferred = -e.get_errno();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        bytes_transferred = -EIO;
    }
    return bytes_transferred;
}

static int planet_readdir(char const *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    ::syslog(LOG_INFO, "%s: path=%s buf=%p offset=%llu", __func__, path, buf, offset);
    int ret = 0;
    try {
        filler(buf, ".", nullptr, 0);
        filler(buf, "..", nullptr, 0);
        for (auto const& entry_name : fs_root.readdir(path))
            if (filler(buf, entry_name.c_str(), nullptr, 0))
                break;
    } catch (planet::exception_errno& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.get_errno();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

static int planet_release(char const *path, struct fuse_file_info *fi)
{
    ::syslog(LOG_INFO, "%s: path=%s fi=%p", __func__, path, fi);
    int ret = 0;
    try {
        planet::handle_t ph = planet::get_handle_from(*fi);
        ::syslog(LOG_INFO, "%s: handle=%d", __func__, ph);
        ret = planet::close(ph);
    } catch (planet::exception_errno& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.get_errno();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

// Install certain file operations
void planet_install_file_operations()
{
    fs_root.install_op<planet::dns_op>();
    fs_root.install_op<planet::tcp_client_op>();
    fs_root.install_op<planet::tcp_server_op>(fs_root);
    fs_root.install_op<planet::packet_socket_op>();
    fs_root.install_op<planet::default_file_op>();
}

// Create initial filesystem structure
void planet_create_initial_fs_structure()
{
    fs_root.mkdir("/ip",            S_IRWXU);
    fs_root.mkdir("/tcp",           S_IRWXU);
    fs_root.mkdir("/eth",           S_IRWXU);
    fs_root.mknod("/eth/lo",        S_IRUSR | S_IWUSR, 0);
    fs_root.mknod("/eth/eth0",      S_IRUSR | S_IWUSR, 0);
    fs_root.mknod("/eth/wlan0",     S_IRUSR | S_IWUSR, 0);
    fs_root.mknod("/eth/enp6s0",    S_IRUSR | S_IWUSR, 0);
    fs_root.mknod("/eth/wlp3s0",    S_IRUSR | S_IWUSR, 0);
    fs_root.mknod("/dns",           S_IRUSR | S_IWUSR, 0);
}

static struct fuse_operations planet_ops{};

int main(int argc, char **argv)
{
    int exit_code = EXIT_SUCCESS;
    try {
        openlog("fuse_planet", LOG_CONS | LOG_PID, LOG_USER);
        ::syslog(LOG_INFO, "fuse_planetfs daemon started");
        planet_install_file_operations();
        planet_create_initial_fs_structure();
        planet_ops.getattr  = planet_getattr;
        planet_ops.mknod    = planet_mknod;
        planet_ops.unlink   = planet_unlink;
        planet_ops.mkdir    = planet_mkdir;
        planet_ops.chmod    = planet_chmod;
        planet_ops.chown    = planet_chown;
        planet_ops.truncate = planet_truncate;
        planet_ops.utimens  = planet_utimens;
        planet_ops.open     = planet_open;
        planet_ops.read     = planet_read;
        planet_ops.write    = planet_write;
        planet_ops.readdir  = planet_readdir;
        planet_ops.release  = planet_release;
        exit_code = fuse_main(argc, argv, &planet_ops, nullptr);
        ::syslog(LOG_INFO, "fuse_planetfs daemon finished");
    } catch (std::exception& e) {
        ::syslog(LOG_ERR, "fatal error occurred: %s", e.what());
        exit_code = EXIT_FAILURE;
    }
    return exit_code;
}
