#ifndef PLANETFS_OPERATIONS_HPP
#define PLANETFS_OPERATIONS_HPP

#include <planet/filesystem.hpp>

#define LOG_EXCEPTION_MSG(e) \
    BOOST_LOG_TRIVIAL(error) << __func__ << ": " << (e).what();

// Core filesystem object
extern planet::filesystem fs;


extern int planet_getattr(char const *, struct stat *);
extern int planet_mknod(char const *, mode_t, dev_t);
extern int planet_unlink(char const *);
extern int planet_mkdir(char const *, mode_t);
extern int planet_rmdir(char const *);
extern int planet_chmod(char const *, mode_t);
extern int planet_chown(char const *, uid_t, gid_t);
extern int planet_truncate(char const *, off_t);
extern int planet_utimens(char const *, struct timespec const [2]);
extern int planet_open(char const *, struct fuse_file_info *);
extern int planet_read(char const *, char *, size_t, off_t, struct fuse_file_info *);
extern int planet_write(char const *, const char *, size_t, off_t, struct fuse_file_info *);
extern int planet_readdir(char const *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);
extern int planet_release(char const *, struct fuse_file_info *);
extern int planet_poll(const char *, struct fuse_file_info *, struct fuse_pollhandle *, unsigned *);


#endif  // PLANETFS_OPERATIONS_HPP
