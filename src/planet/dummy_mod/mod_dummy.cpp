
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <syslog.h>

#define MODULE_NAME "mod_dummy"

using planet::shared_ptr;
using planet::detail::shared_null_ptr;
using planet::path_type;
using planet::file_type;
using planet::fs_entry;
using planet::fs_operation;

extern "C" {

    void planet_mod_init(shared_ptr<planet::core_file_system> fs_root)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: module installed", MODULE_NAME, __PRETTY_FUNCTION__);
    }

    void planet_mod_fin()
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: module uninstalled", MODULE_NAME, __PRETTY_FUNCTION__);
    }

    shared_ptr<fs_operation> planet_mod_new_instance()
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: called", MODULE_NAME, __PRETTY_FUNCTION__);
        return shared_null_ptr;
    }

    int planet_mod_open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: path=%s", MODULE_NAME, __PRETTY_FUNCTION__, path.string().c_str());
        return -EPERM;
    }

    int planet_mod_read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: size=%d, offset=%llu", MODULE_NAME, __PRETTY_FUNCTION__, size, offset);
        return -EPERM;
    }

    int planet_mod_write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: size=%d, offset=%llu", MODULE_NAME, __PRETTY_FUNCTION__, size, offset);
        return -EPERM;
    }

    int planet_mod_release(shared_ptr<fs_entry> file_ent)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: called", MODULE_NAME, __PRETTY_FUNCTION__);
        return -EPERM;
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

    bool planet_mod_is_matching_path(path_type const& path, file_type type)
    {
        ::syslog(LOG_NOTICE, "module=%s: %s: path=%s", MODULE_NAME, __PRETTY_FUNCTION__, path.string().c_str());
        return false;
    }

}   // extern "C"
