
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/dyn_module_op.hpp>

namespace planet {


    dyn_module_op::dyn_module_op(string_type const& mod_name, shared_ptr<core_file_system> fs_root)
            :   mod_name_(mod_name)
    {
        loader_.add_searchdir({"./"});
        loader_.load_module(mod_name_);
        load_all_functions();
        init_(fs_root);
        ::syslog(LOG_NOTICE, "dyn_module_op: ctor: installed module: %s", mod_name_.c_str());
    }

    dyn_module_op::~dyn_module_op()
    {
        try {
            fin_();
        } catch (...) {
            // dtor must not throw any exceptions
        }
        ::syslog(LOG_NOTICE, "dyn_module_op: dtor: uninstalled module: %s", mod_name_.c_str());
    }

    shared_ptr<fs_operation> dyn_module_op::new_instance()
    {
        return new_instance_();
    }

    int dyn_module_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        return open_(file_ent, path);
    }

    int dyn_module_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        return read_(file_ent, buf, size, offset);
    }

    int dyn_module_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        return write_(file_ent, buf, size, offset);
    }

    int dyn_module_op::release(shared_ptr<fs_entry> file_ent)
    {
        return release_(file_ent);
    }

    int dyn_module_op::mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t mode, dev_t dev)
    {
        return mknod_(file_ent, path, mode, dev);
    }

    int dyn_module_op::rmnod(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        return rmnod_(file_ent, path);
    }

    bool dyn_module_op::is_matching_path(path_type const& path, file_type type)
    {
        return matching_path_(path, type);
    }

    void dyn_module_op::load_all_functions()
    {
        init_           = reinterpret_cast<mod_init_t>(loader_.load_symbol("planet_mod_init"));
        fin_            = reinterpret_cast<mod_fin_t>(loader_.load_symbol("planet_mod_fin"));
        new_instance_   = reinterpret_cast<mod_new_instance_t>(loader_.load_symbol("planet_mod_new_instance"));
        open_           = reinterpret_cast<mod_open_t>(loader_.load_symbol("planet_mod_open"));
        read_           = reinterpret_cast<mod_read_t>(loader_.load_symbol("planet_mod_read"));
        write_          = reinterpret_cast<mod_write_t>(loader_.load_symbol("planet_mod_write"));
        release_        = reinterpret_cast<mod_release_t>(loader_.load_symbol("planet_mod_release"));
        mknod_          = reinterpret_cast<mod_mknod_t>(loader_.load_symbol("planet_mod_mknod"));
        rmnod_          = reinterpret_cast<mod_rmnod_t>(loader_.load_symbol("planet_mod_rmnod"));
        matching_path_  = reinterpret_cast<mod_matching_path_t>(loader_.load_symbol("planet_mod_is_matching_path"));
        //
        // install_     = reinterpret_cast<mod_matching_path_t>(::dlsym(handle_, "planet_mod_install"));
        // if (auto err = dlerror())
        //     throw exception_errno(ELIBACC, str(format("%1%: ") % err));
        // uninstall_   = reinterpret_cast<mod_matching_path_t>(::dlsym(handle_, "planet_mod_uninstall"));
        // if (auto err = dlerror())
        //     throw exception_errno(ELIBACC, str(format("%1%: ") % err));
    }


}   // namespace planet
