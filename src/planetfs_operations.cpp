#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/handle.hpp>
#include <planet/filesystem.hpp>
#include <planetfs_operations.hpp>

// Core filesystem object
std::unique_ptr<planet::filesystem> fs;


int planet_getattr(char const *path, struct stat *stbuf)
{
    BOOST_LOG_TRIVIAL(trace)
        << __func__ << ": path=" << path << " stbuf=" << stbuf;
    int ret = 0;
    try {
        std::memset(stbuf, 0, sizeof (struct stat));
        ret = fs->root()->getattr(path, *stbuf);
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
    BOOST_LOG_TRIVIAL(trace)
        << __func__ << ": path=" << path << " mode=" << mode << " dev=" << device;
    int ret = 0;
    try {
        ret = fs->root()->mknod(path, mode, device);
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
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << path;
    int ret = 0;
    try {
        ret = fs->root()->unlink(path);
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
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << path << " mode=" << mode;
    int ret = 0;
    try {
        ret = fs->root()->mkdir(path, 0755);
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
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << path;
    int ret = 0;
    try {
        ret = fs->root()->rmdir(path);
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
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << path << " mode=" << mode;
    int ret = 0;
    try {
        ret = fs->root()->chmod(path, mode);
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
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << path << " uid=" << uid << " gid=" << gid;
    int ret = 0;
    try {
        ret = fs->root()->chown(path, uid, gid);
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
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << path << " offset=" << offset;
    int ret = 0;
    try {
        ret = fs->root()->truncate(path, offset);
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
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << path;
    int ret = 0;
    try {
        ret = fs->root()->utimens(path, tv);
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
    BOOST_LOG_TRIVIAL(trace) << __func__ << ": path=" << path << " fi=" << fi;
    int ret = 0;
    try {
        planet::handle_t ph = fs->root()->open(path);
        planet::set_handle_to(*fi, ph);
        BOOST_LOG_TRIVIAL(debug) << __func__ << ": handle=" << ph;
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
    BOOST_LOG_TRIVIAL(trace)
        << __func__ << ": path=" << path
        << " size=" << size << " offset=" << offset << " fi=" << fi;
    int bytes_received;
    try {
        planet::handle_t ph = planet::get_handle_from(*fi);
        bytes_received = fs->root()->read(ph, buf, size, offset);
        BOOST_LOG_TRIVIAL(debug) << __func__ << ": handle=" << ph << " bytes_received=" << bytes_received;
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
    BOOST_LOG_TRIVIAL(trace)
        << __func__ << ": path=" << path
        << " size=" << size << " offset=" << offset << " fi=" << fi;
    int bytes_transferred;
    try {
        planet::handle_t ph = planet::get_handle_from(*fi);
        bytes_transferred = fs->root()->write(ph, buf, size, offset);
        BOOST_LOG_TRIVIAL(debug) << __func__ << ": handle=" << ph << " bytes_transferred=" << bytes_transferred;
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
    BOOST_LOG_TRIVIAL(trace) << __func__ << " path=" << path << " offset=" << offset;
    int ret = 0;
    try {
        filler(buf, ".", nullptr, 0);
        filler(buf, "..", nullptr, 0);
        for (auto const& entry_name : fs->root()->readdir(path))
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
    BOOST_LOG_TRIVIAL(trace) << __func__ << " path=" << path << " fi=" << fi;
    int ret = 0;
    try {
        planet::handle_t ph = planet::get_handle_from(*fi);
        BOOST_LOG_TRIVIAL(debug) << __func__ << " handle=" << ph << " path=" << path << " fi=" << fi;
        ret = fs->root()->close(ph);
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
    try {
        ret = fs->root()->poll(path, fi, ph, reventsp);
    } catch (std::system_error& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -e.code().value();
    } catch (std::exception& e) {
        LOG_EXCEPTION_MSG(e);
        ret = -EIO;
    }
    return ret;
}
