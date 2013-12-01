#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <iostream>
#include <cstdlib>
//#include <planet/net/dns/installer.hpp>
//#include <planet/net/tcp/installer.hpp>
//#include <planet/net/eth/installer.hpp>
#include <planetfs_operations.hpp>
#include <syslog.h>
#include <signal.h>

#define PLANETFS_NAME "fuse_planetfs"

// Install certain file operations
void planetfs_install_entry_ops()
{
    typedef planet::core_file_system::priority priority;

    // static module loading
    //fs.root()->install_op<planet::net::dns::installer>(priority::normal);

    // dynamic module loading
    // TODO: FIX BUGS HERE:
    //fs.root()->install_module(priority::normal, "mod_net_dns");

    //fs.root()->install_op<planet::net::tcp::installer>(priority::normal);
    //fs.root()->install_op<planet::net::eth::installer>(priority::normal);
}

// Create initial filesystem structure
void planetfs_create_initial_fs_structure()
{
    fs.root()->mkdir("/ip",    S_IRWXU);
    fs.root()->mkdir("/tcp",   S_IRWXU);
    fs.root()->mkdir("/eth",   S_IRWXU);
    fs.root()->mknod("/dns",   S_IRUSR | S_IWUSR, 0);
}

static struct fuse_operations planetentry_op{};

int main(int argc, char **argv)
{
    int exit_code = EXIT_FAILURE;
    try {
        openlog(PLANETFS_NAME, LOG_CONS | LOG_PID, LOG_USER);
        ::syslog(LOG_INFO, "%s daemon started", PLANETFS_NAME);

        // Initialize filesystem
        planetfs_install_entry_ops();
        planetfs_create_initial_fs_structure();

        // Set system call functions
        planetentry_op.getattr    =   planet_getattr;
        planetentry_op.mknod      =   planet_mknod;
        planetentry_op.unlink     =   planet_unlink;
        planetentry_op.mkdir      =   planet_mkdir;
        planetentry_op.rmdir      =   planet_rmdir;
        planetentry_op.chmod      =   planet_chmod;
        planetentry_op.chown      =   planet_chown;
        planetentry_op.truncate   =   planet_truncate;
        planetentry_op.utimens    =   planet_utimens;
        planetentry_op.open       =   planet_open;
        planetentry_op.read       =   planet_read;
        planetentry_op.write      =   planet_write;
        planetentry_op.readdir    =   planet_readdir;
        planetentry_op.release    =   planet_release;

        // Start userspace filesystem
        exit_code = ::fuse_main(argc, argv, &planetentry_op, nullptr);

    } catch (std::exception& e) {
        ::syslog(LOG_ERR, "fatal error: %s", e.what());
    } catch (...) {
        ::syslog(LOG_ERR, "unknown fatal error");
    }
    ::syslog(LOG_INFO, "%s daemon finished", PLANETFS_NAME);
    return exit_code;
}
