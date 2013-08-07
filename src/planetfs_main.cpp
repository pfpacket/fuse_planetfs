#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <iostream>
#include <cstdlib>
#include <planet/net/dns/installer.hpp>
#include <planet/net/tcp/installer.hpp>
#include <planet/net/eth/installer.hpp>
#include <planetfs_operations.hpp>
#include <syslog.h>
#include <signal.h>

#define PLANETFS_NAME "fuse_planetfs"

// Install certain file operations
void planetfs_install_fs_operations()
{
    typedef planet::core_file_system::priority priority;
    // static module loading
    //fs_root->install_op<planet::net::dns::installer>(priority::normal, fs_root);
    // dynamic module loading
    fs_root->install_module(priority::normal, "mod_net_dns.so");
    fs_root->install_op<planet::net::tcp::installer>(priority::normal);
    fs_root->install_op<planet::net::eth::installer>(priority::normal);
}

// Create initial filesystem structure
void planetfs_create_initial_fs_structure()
{
    fs_root->mkdir("/ip",    S_IRWXU);
    fs_root->mkdir("/tcp",   S_IRWXU);
    fs_root->mkdir("/eth",   S_IRWXU);
    fs_root->mknod("/dns",   S_IRUSR | S_IWUSR, 0);
}

void planetfs_sig_handler(int sig)
{
    if (sig == SIGABRT)
        ::syslog(LOG_ERR, "SIGABRT caught: may be a bug");
    else
        ::syslog(LOG_ERR, "unknown signal caught: %d", sig);
}

static struct fuse_operations planetfs_ops{};

int main(int argc, char **argv)
{
    int exit_code = EXIT_SUCCESS;
    if (signal(SIGABRT, &planetfs_sig_handler) == SIG_ERR) {
        perror("signal registering: ");
        std::exit(EXIT_FAILURE);
    }
    try {
        openlog(PLANETFS_NAME, LOG_CONS | LOG_PID, LOG_USER);
        ::syslog(LOG_INFO, "%s daemon started", PLANETFS_NAME);
        planetfs_install_fs_operations();
        planetfs_create_initial_fs_structure();
        fs_root->uninstall_op<planet::net::eth::raw_op>();
        fs_root->uninstall_op<planet::net::eth::dir_op>();
        planetfs_ops.getattr    =   planet_getattr;
        planetfs_ops.mknod      =   planet_mknod;
        planetfs_ops.unlink     =   planet_unlink;
        planetfs_ops.mkdir      =   planet_mkdir;
        planetfs_ops.rmdir      =   planet_rmdir;
        planetfs_ops.chmod      =   planet_chmod;
        planetfs_ops.chown      =   planet_chown;
        planetfs_ops.truncate   =   planet_truncate;
        planetfs_ops.utimens    =   planet_utimens;
        planetfs_ops.open       =   planet_open;
        planetfs_ops.read       =   planet_read;
        planetfs_ops.write      =   planet_write;
        planetfs_ops.readdir    =   planet_readdir;
        planetfs_ops.release    =   planet_release;
        exit_code = fuse_main(argc, argv, &planetfs_ops, nullptr);
    } catch (std::exception& e) {
        ::syslog(LOG_ERR, "fatal error occurred: %s", e.what());
        exit_code = EXIT_FAILURE;
    } catch (...) {
        ::syslog(LOG_ERR, "unknown fatal error occurred");
        exit_code = EXIT_FAILURE;
    }
    planet::core_file_system::destroy(fs_root);
    ::syslog(LOG_INFO, "%s daemon finished", PLANETFS_NAME);
    return exit_code;
}
