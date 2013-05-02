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
#include <planet/planet_handle.hpp>
#include <planet/operation_layer.hpp>
#include <syslog.h>

#define LOG_EXCEPTION_MSG(e) \
    ::syslog(LOG_INFO, "%s: %s", __func__, e.what());

// Core filesystem object
planet::core_file_system fs_root;

static int planet_getattr(char const *path, struct stat *stbuf)
{
    ::syslog(LOG_INFO, "%s: path=%s stbuf=%p", __func__, path, stbuf);
    int ret = 0;
    try {
        fs_root.getattr(path, *stbuf);
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

static int planet_mkdir(char const *path, mode_t mode)
{
    ::syslog(LOG_INFO, "%s: path=%s mode=%o", __func__, path, mode);
    int ret = 0;
    try {
        ret = fs_root.mkdir(path, 0755 | S_IFDIR);
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
    fs_root.install_op<planet::default_file_op>();
}

// Create initial filesystem structure
void planet_create_initial_fs_structure()
{
    fs_root.mkdir("/eth",       S_IFDIR | S_IRWXU);
    fs_root.mkdir("/ip",        S_IFDIR | S_IRWXU);
    fs_root.mkdir("/tcp",       S_IFDIR | S_IRWXU);
    fs_root.mknod("/dns",       S_IFREG | S_IRWXU, 0);
    fs_root.mknod("/eth/eth0",  S_IFREG | S_IRWXU, 0);
    fs_root.mknod("/eth/wlan0", S_IFREG | S_IRWXU, 0);
}

static struct fuse_operations planet_ops{};

int main(int argc, char **argv)
{
    int exit_code = EXIT_SUCCESS;
    try {
        openlog("fuse_planet", LOG_CONS | LOG_PID, LOG_USER);
        planet_install_file_operations();
        planet_create_initial_fs_structure();
        planet_ops.getattr  = planet_getattr;
        planet_ops.mknod    = planet_mknod;
        planet_ops.mkdir    = planet_mkdir;
        planet_ops.open     = planet_open;
        planet_ops.read     = planet_read;
        planet_ops.write    = planet_write;
        planet_ops.readdir  = planet_readdir;
        planet_ops.release  = planet_release;
        exit_code = fuse_main(argc, argv, &planet_ops, nullptr);
    } catch (std::exception& e) {
        ::syslog(LOG_ERR, "fatal error occurred: %s", e.what());
        exit_code = EXIT_FAILURE;
    }
    return exit_code;
}
