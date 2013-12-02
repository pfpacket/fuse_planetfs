
#include <planet/common.hpp>
#include <planet/module_ops_type.hpp>

namespace planet {


    void module_ops_type::load_all_functions()
    {
        init_       = loader_.load_func_ptr<mod_init_t>("planet_mod_init");
        fin_        = loader_.load_func_ptr<mod_fin_t>("planet_mod_fin");
        create_op_  = loader_.load_func_ptr<mod_create_op_t>("planet_mod_create_op");
        mknod_      = loader_.load_func_ptr<mod_mknod_t>("planet_mod_mknod");
        rmnod_      = loader_.load_func_ptr<mod_rmnod_t>("planet_mod_rmnod");
        match_path_ = loader_.load_func_ptr<mod_match_path_t>("planet_mod_match_path");
    }

    module_ops_type::module_ops_type(string_type const& module_name)
        :   fs_ops_type(module_name)
    {
        loader_.add_searchdir({"./"});
        loader_.load_module(module_name);
        load_all_functions();
        ::syslog(LOG_NOTICE, "module_ops_type: ctor: installed module: %s", module_name.c_str());
    }

    int module_ops_type::install(shared_ptr<core_file_system> fs_root)
    {
        return init_(fs_root);
    }

    int module_ops_type::uninstall(shared_ptr<core_file_system> fs_root)
    {
        return fin_(fs_root);
    }

    shared_ptr<entry_op> module_ops_type::create_op(shared_ptr<core_file_system> fs_root)
    {
        return create_op_(fs_root);
    }

    int module_ops_type::mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t mode, dev_t dev)
    {
        return mknod_(file_ent, path, mode, dev);
    }

    int module_ops_type::rmnod(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        return rmnod_(file_ent, path);
    }

    bool module_ops_type::match_path(path_type const& path, file_type type)
    {
        return match_path_(path, type);
    }


}   // namespace planet
