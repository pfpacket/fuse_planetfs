
#include <planet/common.hpp>
#include <planet/net/eth/dir_op.hpp>
#include <planet/net/eth/raw_op.hpp>
#include <planet/utils.hpp>

#define MODULE_NAME "mod_net_eth"

using namespace planet;
using priority = core_file_system::priority;
using dir_type = planet::net::eth::dir_type;
using ether_type = planet::net::eth::raw_type;

static planet::shared_ptr<ether_type> ether = nullptr;

extern "C" {


    int planet_mod_init(shared_ptr<planet::core_file_system> fs_root)
    {
        fs_root->install_ops<dir_type>(priority::normal);
        ether = planet::make_shared<ether_type>();
        return ether ? ether->install(fs_root) : -ENOSPC;
    }

    int planet_mod_fin(shared_ptr<core_file_system> fs_root)
    {
        fs_root->uninstall_ops(dir_type::type_name);
        return ether->uninstall(fs_root);
    }

    shared_ptr<entry_op> planet_mod_create_op(shared_ptr<core_file_system> fs_root)
    {
        return ether->create_op(fs_root);
    }

    int planet_mod_mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t mode, dev_t dev)
    {
        return ether->mknod(file_ent, path, mode, dev);
    }

    int planet_mod_rmnod(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        return ether->rmnod(file_ent, path);
    }

    bool planet_mod_match_path(path_type const& path, file_type type)
    {
        return ether->match_path(path, type);
    }


}   // extern "C"
