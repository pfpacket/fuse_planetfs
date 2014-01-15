
#include <planet/common.hpp>
#include <planet/net/dns/resolver_op.hpp>

#define MODULE_NAME "mod_net_dns"

using planet::shared_ptr;
using planet::detail::shared_null_ptr;
using planet::path_type;
using planet::file_type;
using planet::fs_entry;
using planet::entry_op;
using planet::core_file_system;
using resolver_type = planet::net::dns::resolver_type;

//static auto resolver = planet::make_shared<resolver_type>();
static resolver_type resolver;

extern "C" {


    int planet_mod_init(shared_ptr<planet::core_file_system> fs_root)
    {
        return resolver.install(fs_root);
    }

    int planet_mod_fin(shared_ptr<core_file_system> fs_root)
    {
        return resolver.uninstall(fs_root);
    }

    shared_ptr<entry_op> planet_mod_create_op(shared_ptr<core_file_system> fs_root)
    {
        return resolver.create_op(fs_root);
    }

    int planet_mod_mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t mode, dev_t dev)
    {
        return resolver.mknod(file_ent, path, mode, dev);
    }

    int planet_mod_rmnod(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        return resolver.rmnod(file_ent, path);
    }

    bool planet_mod_match_path(path_type const& path, file_type type)
    {
        return resolver.match_path(path, type);
    }


}   // extern "C"
