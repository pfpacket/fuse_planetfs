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
#include <planet/dns/dns_op.hpp>
#include <planet/tcp/client_op.hpp>
#include <planet/tcp/server_op.hpp>
#include <planet/eth/raw_op.hpp>
#include <planetfs_operations.hpp>
#include <syslog.h>


// Install certain file operations
void planet_install_file_operations()
{
    fs_root.install_op<planet::net::dns::dns_op>();
    fs_root.install_op<planet::net::tcp::client_op>();
    fs_root.install_op<planet::net::tcp::server_op>(fs_root);
    fs_root.install_op<planet::net::eth::raw_op>();
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
        planet_ops.rmdir    = planet_rmdir;
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
