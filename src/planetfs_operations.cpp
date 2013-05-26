#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <planetfs_operations.hpp>
#include <planet/operation_layer.hpp>
#include <planet/planet_handle.hpp>
#include <syslog.h>

// Core filesystem object
planet::core_file_system fs_root(S_IRWXU);


int planet_getattr(char const *path, struct stat *stbuf)
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

int planet_mknod(char const *path, mode_t mode, dev_t device)
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

int planet_unlink(char const *path)
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

int planet_mkdir(char const *path, mode_t mode)
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

int planet_chmod(char const *path, mode_t mode)
{
    return 0;
}

int planet_chown(char const *path, uid_t uid, gid_t gid)
{
    return 0;
}

int planet_truncate(char const *path, off_t offset)
{
    return 0;
}

int planet_utimens(char const* path, struct timespec const tv[2])
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

int planet_open(char const *path, struct fuse_file_info *fi)
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

int planet_read(char const *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
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

int planet_write(char const *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
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

int planet_readdir(char const *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
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

int planet_release(char const *path, struct fuse_file_info *fi)
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