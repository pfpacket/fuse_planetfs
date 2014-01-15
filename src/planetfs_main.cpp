#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <iostream>
#include <cstdlib>
#include <planet/net/dns/installer.hpp>
#include <planet/net/tcp/installer.hpp>
#include <planet/net/eth/installer.hpp>
#include <planet/module_loader/module_loader.hpp>
#include <planetfs_operations.hpp>
#include <signal.h>

#include <boost/log/sinks.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/utility/empty_deleter.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#define PLANETFS_NAME "fuse_planetfs"
namespace logging   = boost::log;
namespace expr      = boost::log::expressions;
namespace sinks     = boost::log::sinks;
namespace keywords  = boost::log::keywords;

void planetfs_initialize_log(std::string const& exec)
{
    logging::add_file_log(
        keywords::auto_flush    = true,
        keywords::open_mode     = std::ios::app,
        //keywords::target        = "log",
        keywords::file_name     = "logs/" + exec + "_%Y%m%d.log",
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::max_size      = 10LL * 1024 * 1024 * 1024,
        keywords::format        = expr::format("%1%\t[%2%]\t%3%")
            % expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
            % logging::trivial::severity % expr::message
    );
    logging::add_common_attributes();
}

void planetfs_log_init_logo()
{
    BOOST_LOG_TRIVIAL(info) << "+---------------------------------+";
    BOOST_LOG_TRIVIAL(info) << "|     Starting fuse_planetfs      |";
    BOOST_LOG_TRIVIAL(info) << "+---------------------------------+";
}

void planetfs_log_fin_logo()
{
    BOOST_LOG_TRIVIAL(info) << "+---------------------------------+";
    BOOST_LOG_TRIVIAL(info) << "|     Finishing fuse_planetfs     |";
    BOOST_LOG_TRIVIAL(info) << "+---------------------------------+";
}

// Install certain file operations
void planetfs_install_file_operations()
{
    typedef planet::core_file_system::priority priority;

    // dynamic module loading
    // TODO: FIX BUGS HERE: Maybe mod_net_dns causes this bug
    //fs.root()->install_module(priority::normal, "mod_net_dns");

    // THERE SEEMS TO BE NO BUGS HERE
    fs.root()->install_module(priority::normal, "mod_dummy");

    // static module loading
    fs.root()->install_ops<planet::net::dns::installer>(priority::low);
    fs.root()->install_ops<planet::net::eth::installer>(priority::low);
    fs.root()->install_ops<planet::net::tcp::installer>(priority::low);
    fs.root()->install_ops<planet::module_loader>(priority::normal);

    // Uninstall operation
    //fs.root()->uninstall_ops("planet.net.dns.installer");
}

// Create initial filesystem structure
void planetfs_create_initial_fs_structure()
{
    fs.root()->mkdir("/ip",    S_IRWXU);
    fs.root()->mkdir("/tcp",   S_IRWXU);
    fs.root()->mkdir("/eth",   S_IRWXU);
    fs.root()->mknod("/dns",   S_IRUSR | S_IWUSR, 0);
}

void planetfs_print_installed_operations()
{
    std::cout << "Installed operations:" << std::endl;
    for (auto&& info : fs.root()->get_ops_db().info())
        std::cout << '\t' << std::get<0>(info) << " / priority=" << std::get<1>(info) << std::endl;
}

static struct fuse_operations planetfs_ops{};

int main(int argc, char **argv)
{
    int exit_code = EXIT_FAILURE;
    planetfs_initialize_log(PLANETFS_NAME);
    try {
        planetfs_log_init_logo();

        // Initialize filesystem
        planetfs_install_file_operations();
        planetfs_create_initial_fs_structure();
        //planetfs_print_installed_operations();

        // Set system call functions
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
        planetfs_ops.poll       =   planet_poll;

        // Start the userspace filesystem
        exit_code = ::fuse_main(argc, argv, &planetfs_ops, nullptr);

    } catch (std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "fatal error: " << e.what();
    } catch (...) {
        BOOST_LOG_TRIVIAL(fatal) << "unknown fatal error";
    }
    planetfs_log_fin_logo();
    logging::core::get()->remove_all_sinks();
    return exit_code;
}
