#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <fuse.h>
#include <fusecpp.hpp>

// Root of this filesystem
fusecpp::directory root{"/"};

static int pnetfs_getattr(char const *path, struct stat *stbuf)
{
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if (auto ptr = fusecpp::search_entry(root, path)) {
        syslog(LOG_INFO, "pnetfs_getattr: path=%s,ptr_cnt=%ld", path, ptr.use_count());
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

static int pnetfs_mknod(char const *path, mode_t mode, dev_t device)
{
    syslog(LOG_INFO, "pnetfs_mknod: creating %s mode=%o", path, mode);

    fusecpp::path_type p(path);
    if (fusecpp::search_file(root, path))
        return -EEXIST;
    auto dir_ptr = fusecpp::search_directory(root, p.parent_path());
    if (!dir_ptr)
        return -ENOENT;
    dir_ptr->create_file(p, mode);
    return 0;
}

static int pnetfs_mkdir(char const *path, mode_t mode)
{
    syslog(LOG_INFO, "pnetfs_mkdir: creating %s mode=%o dirmode=%o", path, mode, mode | S_IFDIR);

    fusecpp::path_type p(path);
    if (fusecpp::search_directory(root, p))
        return -EEXIST;
    auto dir_ptr = fusecpp::search_directory(root, p.parent_path());
    if (!dir_ptr)
        return -ENOENT;
    dir_ptr->create_directory(p, mode | S_IFDIR);
    return 0;
}

static int pnetfs_unlink(char const *path)
{
    fusecpp::path_type p(path);
    if (auto ptr = fusecpp::search_directory(root, p.parent_path())) {
        ptr->remove_entry(path);
        return 0;
    }
    return -ENOENT;
}

static int pnetfs_open(char const *path, struct fuse_file_info *fi)
{
    syslog(LOG_INFO, "pnetfs_open: opening %s", path);

    auto ptr = fusecpp::search_file(root, path);
    if (!ptr)
        return -ENOENT;
    fi->fh = reinterpret_cast<decltype(fi->fh)>(&(*ptr));
    return 0;
}

static int pnetfs_read(char const *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    syslog(LOG_INFO, "pnetfs_read: reading %s size=%u, offset=%lld", path, size, offset);

    fusecpp::file *file = reinterpret_cast<fusecpp::file *>(fi->fh);
    size_t file_length = file->data().size();
    if (offset > file_length)
        return 0;
    if (offset + size > file_length)
        size = file_length - offset;
    std::memcpy(buf, file->data().data() + offset, size);
    return size;
}

static int pnetfs_write(char const *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    syslog(LOG_INFO, "pnetfs_write: writing %s size=%u, offset=%lld", path, size, offset);

    fusecpp::file *file = reinterpret_cast<fusecpp::file *>(fi->fh);
    try {
        file->data().resize(offset + size);
    } catch (std::exception& e) {
        return -ENOSPC;
    }
    std::memcpy(fusecpp::get_file_data(*file) + offset, buf, size);
    return size;
}

static int pnetfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    syslog(LOG_INFO, "pnetfs_readdir: reading %s buf=%p, offset=%lld", path, buf, offset);

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

static struct fuse_operations pnetfs_ops{};

int main(int argc, char **argv)
{
    pnetfs_ops.getattr  = pnetfs_getattr;
    pnetfs_ops.mknod    = pnetfs_mknod;
    pnetfs_ops.mkdir    = pnetfs_mkdir;
    pnetfs_ops.unlink   = pnetfs_unlink;
    pnetfs_ops.open     = pnetfs_open;
    pnetfs_ops.read     = pnetfs_read;
    pnetfs_ops.write    = pnetfs_write;
    pnetfs_ops.readdir  = pnetfs_readdir;
    openlog("fuse_pnetfs", LOG_CONS | LOG_PID, LOG_USER);
    return fuse_main(argc, argv, &pnetfs_ops, NULL);
}
