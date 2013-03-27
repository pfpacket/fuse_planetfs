#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <fcntl.h>
#include <fuse.h>
#include <fusecpp.hpp>

// Root of this filesystem
fusecpp::directory root{"/"};

static int pnetfs_getattr(char const *path, struct stat *stbuf)
{
    return 0;
}

static int pnetfs_mknod(char const *path, mode_t mode, dev_t device)
{
    fusecpp::path_type p(path);
    auto ptr = fusecpp::search_directory(root, p.parent_path());
    if (!ptr)
        return -ENOENT;
    ptr->create_file(p);
    return 0;
}

static int pnetfs_open(char const *path, struct fuse_file_info *fi)
{
    return 0;
}

static int pnetfs_read(char const *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    return 0;
}

static int pnetfs_write(char const *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    return 0;
}

static int pnetfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    auto dir = fusecpp::search_directory(root, path);
    if (!dir)
        return -ENOENT;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    for (auto ptr : dir->get_entries())
        if (ptr->is_file() && filler(buf, fusecpp::file_cast(ptr)->path().c_str(), NULL, 0))
          break;
    return 0;
}

static struct fuse_operations pnetfs_ops{};

int main(int argc, char **argv)
{
    pnetfs_ops.getattr  = pnetfs_getattr;
    pnetfs_ops.mknod    = pnetfs_mknod;
    pnetfs_ops.open     = pnetfs_open;
    pnetfs_ops.read     = pnetfs_read;
    pnetfs_ops.write    = pnetfs_write;
    pnetfs_ops.readdir  = pnetfs_readdir;
    return fuse_main(argc, argv, &pnetfs_ops, NULL);
}
