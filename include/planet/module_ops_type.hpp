#ifndef PLANET_MODULE_OPS_TYPE_HPP
#define PLANET_MODULE_OPS_TYPE_HPP

#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/dl_loader.hpp>
#include <planet/fs_ops_type.hpp>

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

        void load_all_functions();

    public:
        module_ops_type(string_type const& module_name);
        // Load a module from library paths including `paths`
        module_ops_type(string_type const& module_name, std::vector<string_type> paths);

        int install(shared_ptr<core_file_system> fs_root);

        int uninstall(shared_ptr<core_file_system> fs_root);

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override;

        int mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t mode, dev_t dev) override;

        int rmnod(shared_ptr<fs_entry> file_ent, path_type const& path) override;

        bool match_path(path_type const& path, file_type type) override;
    };


}   // namespace planet

#endif  // PLANET_MODULE_OPS_TYPE_HPP
