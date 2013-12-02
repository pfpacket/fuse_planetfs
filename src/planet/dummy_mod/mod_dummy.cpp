
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <syslog.h>

#define MODULE_NAME "mod_dummy"

using planet::shared_ptr;
using planet::detail::shared_null_ptr;
using planet::path_type;
using planet::file_type;
using planet::fs_entry;
using planet::entry_op;
using planet::core_file_system;

extern "C" {


    int planet_mod_init(shared_ptr<core_file_system> fs_root)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: module installed", MODULE_NAME, __PRETTY_FUNCTION__);
        return 0;
    }

    int planet_mod_fin(shared_ptr<core_file_system> fs_root)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: module uninstalled", MODULE_NAME, __PRETTY_FUNCTION__);
        return 0;
    }

    shared_ptr<entry_op> planet_mod_create_op(shared_ptr<core_file_system> fs_root)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: called", MODULE_NAME, __PRETTY_FUNCTION__);
        return shared_null_ptr;
    }

    int planet_mod_mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t mode, dev_t dev)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: path=%s", MODULE_NAME, __PRETTY_FUNCTION__, path.string().c_str());
        return -EPERM;
    }

    int planet_mod_rmnod(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: path=%s", MODULE_NAME, __PRETTY_FUNCTION__, path.string().c_str());
        return -EPERM;
    }

    bool planet_mod_match_path(path_type const& path, file_type type)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: path=%s", MODULE_NAME, __PRETTY_FUNCTION__, path.string().c_str());
        return false;
    }


}   // extern "C"
