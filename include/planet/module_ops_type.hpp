#ifndef PLANET_MODULE_OPS_TYPE_HPP
#define PLANET_MODULE_OPS_TYPE_HPP

#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/dl_loader.hpp>

namespace planet {


    class module_ops_type : public fs_ops_type {
    private:
        typedef int (*mod_init_t)(shared_ptr<core_file_system>);
        typedef int (*mod_fin_t)(shared_ptr<core_file_system>);
        typedef shared_ptr<entry_op> (*mod_create_op_t)(shared_ptr<core_file_system>);
        typedef int (*mod_mknod_t)(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t);
        typedef int (*mod_rmnod_t)(shared_ptr<fs_entry>, path_type const&);
        typedef bool (*mod_match_path_t)(path_type const&, file_type);

        dl_loader loader_;

        // loaded module's functions
        mod_init_t          init_;
        mod_fin_t           fin_;
        mod_create_op_t     create_op_;
        mod_mknod_t         mknod_;
        mod_rmnod_t         rmnod_;
        mod_match_path_t    match_path_;

        void load_all_functions()
        {
            init_       = loader_.load_func_ptr<mod_init_t>("planet_mod_init");
            fin_        = loader_.load_func_ptr<mod_fin_t>("planet_mod_fin");
            create_op_  = loader_.load_func_ptr<mod_create_op_t>("planet_mod_create_op");
            mknod_      = loader_.load_func_ptr<mod_mknod_t>("planet_mod_mknod");
            rmnod_      = loader_.load_func_ptr<mod_rmnod_t>("planet_mod_rmnod");
            match_path_ = loader_.load_func_ptr<mod_match_path_t>("planet_mod_match_path");
        }

    public:
        module_ops_type(string_type const& module_name)
            :   fs_ops_type(module_name)
        {
            loader_.add_searchdir({"./"});
            loader_.load_module(module_name);
            load_all_functions();
            ::syslog(LOG_NOTICE, "dyn_module_op: ctor: installed module: %s", module_name.c_str());
        }

        virtual int install(shared_ptr<core_file_system> fs_root)
        {
            return init_(fs_root);
        }

        virtual int uninstall(shared_ptr<core_file_system> fs_root)
        {
            return fin_(fs_root);
        }

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override
        {
            return create_op_(fs_root);
        }

        int mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t mode, dev_t dev) override
        {
            return mknod_(file_ent, path, mode, dev);
        }

        int rmnod(shared_ptr<fs_entry> file_ent, path_type const& path) override
        {
            return rmnod_(file_ent, path);
        }

        bool match_path(path_type const& path, file_type type) override
        {
            return match_path_(path, type);
        }
    };


}   // namespace planet

#endif  // PLANET_MODULE_OPS_TYPE_HPP
