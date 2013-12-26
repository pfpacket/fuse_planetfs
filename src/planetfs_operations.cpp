#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/handle.hpp>
#include <planet/filesystem.hpp>
#include <planetfs_operations.hpp>
#include <syslog.h>

// Core filesystem object
planet::filesystem fs(S_IRWXU);


int planet_getattr(char const *path, struct stat *stbuf)
{
    ::syslog(LOG_INFO, "%s: path=%s stbuf=%p", __func__, path, stbuf);
    int ret = 0;
    try {
        std::memset(stbuf, 0, sizeof (struct stat));
        ret = fs.root()->getattr(path, *stbuf);
        stbuf->st_uid = ::getuid();
        stbuf->st_gid = ::getgid();
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_mknod(char const *path, mode_t mode, dev_t device)
{
    ::syslog(LOG_INFO, "%s: path=%s mode=%o %lld", __func__, path, mode, device);
    int ret = 0;
    try {
        ret = fs.root()->mknod(path, mode, device);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_unlink(char const *path)
{
    ::syslog(LOG_INFO, "%s: path=%s", __func__, path);
    int ret = 0;
    try {
        ret = fs.root()->unlink(path);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_mkdir(char const *path, mode_t mode)
{
    ::syslog(LOG_INFO, "%s: path=%s mode=%o", __func__, path, mode);
    int ret = 0;
    try {
        ret = fs.root()->mkdir(path, 0755);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_rmdir(char const *path)
{
    ::syslog(LOG_INFO, "%s: path=%s", __func__, path);
    int ret = 0;
    try {
        ret = fs.root()->rmdir(path);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_chmod(char const *path, mode_t mode)
{
    ::syslog(LOG_INFO, "%s: path=%s mode=%o", __func__, path, mode);
    int ret = 0;
    try {
        ret = fs.root()->chmod(path, mode);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_chown(char const *path, uid_t uid, gid_t gid)
{
    planet::syslog_fmt(LOG_INFO, planet::format
        ("%1%: path=%2% uid=%3% gid=%4%") % __func__ % path % uid % gid);
    int ret = 0;
    try {
        ret = fs.root()->chown(path, uid, gid);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_truncate(char const *path, off_t offset)
{
    ::syslog(LOG_INFO, "%s: path=%s offset=%lld", __func__, path, offset);
    int ret = 0;
    try {
        if (auto entry = fs.root()->get_entry_of(path))
            planet::file_cast(entry)->data().resize(offset);
        else
            ret = -ENOENT;
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_utimens(char const* path, struct timespec const tv[2])
{
    ::syslog(LOG_INFO, "%s: path=%s", __func__, path);
    int ret = 0;
    try {
        namespace ch = std::chrono;
        if (auto entry = fs.root()->get_entry_of(path)) {
            ch::nanoseconds nano_access(tv[0].tv_nsec), nano_mod(tv[1].tv_nsec);
            ch::seconds sec_access(tv[0].tv_sec), sec_mod(tv[1].tv_sec);
            planet::st_inode new_inode = entry->inode();
            new_inode.atime = decltype(new_inode.atime)
                (ch::duration_cast<decltype(new_inode.atime)::duration>(nano_access + sec_access));
            new_inode.mtime = decltype(new_inode.atime)
                (ch::duration_cast<decltype(new_inode.atime)::duration>(nano_mod + sec_mod));
            entry->inode(new_inode);
        } else
            ret = -ENOENT;
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_open(char const *path, struct fuse_file_info *fi)
{
    ::syslog(LOG_INFO, "%s: path=%s fi=%p", __func__, path, fi);
    int ret = 0;
    try {
        planet::handle_t ph = fs.root()->open(path);
        planet::set_handle_to(*fi, ph);
        ::syslog(LOG_INFO, "%s: handle=%d", __func__, ph);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_read(char const *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int bytes_received;
    try {
        planet::handle_t ph = planet::get_handle_from(*fi);
        ::syslog(LOG_INFO,
            "%s: handle=%d path=%s buf=%p size=%d offset=%llu", __func__, ph, path, buf, size, offset);
        bytes_received = fs.root()->read(ph, buf, size, offset);
        ::syslog(LOG_INFO, "%s: handle=%d bytes_received=%d", __func__, ph, bytes_received);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        bytes_received = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        bytes_received = -EIO;
    }
    return bytes_received;
}

int planet_write(char const *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int bytes_transferred;
    try {
        planet::handle_t ph = planet::get_handle_from(*fi);
        ::syslog(LOG_INFO,
            "%s: handle=%d path=%s buf=%p size=%d offset=%llu", __func__, ph, path, buf, size, offset);
        bytes_transferred = fs.root()->write(ph, buf, size, offset);
        ::syslog(LOG_INFO, "%s: handle=%d bytes_transferred=%d", __func__, ph, bytes_transferred);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        bytes_transferred = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        bytes_transferred = -EIO;
    }
    return bytes_transferred;
}

int planet_readdir(char const *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    ::syslog(LOG_INFO, "%s: path=%s buf=%p offset=%llu", __func__, path, buf, offset);
    int ret = 0;
    try {
        filler(buf, ".", nullptr, 0);
        filler(buf, "..", nullptr, 0);
        for (auto const& entry_name : fs.root()->readdir(path))
            if (filler(buf, entry_name.c_str(), nullptr, 0))
                break;
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_release(char const *path, struct fuse_file_info *fi)
{
    int ret = 0;
    try {
        planet::handle_t ph = planet::get_handle_from(*fi);
        ::syslog(LOG_INFO, "%s: handle=%d path=%s fi=%p", __func__, ph, path, fi);
        ret = fs.root()->close(ph);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}

int planet_poll(const char *path, struct fuse_file_info *fi, struct fuse_pollhandle *ph, unsigned *reventsp)
{
    int ret = 0;
    //planet::syslog_fmt(LOG_NOTICE, planet::format(
    //    "%s: handle=%d, fi=%p ph=%p reventsp=%p") % __func__ % planet::get_handle_from(*fi) % fi % ph % reventsp);
    try {
        ret = fs.root()->poll(path, fi, ph, reventsp);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}
