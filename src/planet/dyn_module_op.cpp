
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/dyn_module_op.hpp>

namespace planet {


    dyn_module_op::dyn_module_op(std::string const& module_path, shared_ptr<core_file_system> fs_root)
            :   mod_name_(module_path),
                handle_(dlopen(module_path.c_str(), RTLD_LAZY))
    {
        if (!handle_)
            throw std::runtime_error(dlerror());
        reload();
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
        ::dlclose(handle_);
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

    void dyn_module_op::reload()
    {
        init_           = reinterpret_cast<mod_init_t>(dlsym(handle_, "planet_mod_init"));
        if (auto err = dlerror())
            throw exception_errno(ELIBACC, str(format("%1%: ") % err).c_str());
        fin_            = reinterpret_cast<mod_fin_t>(dlsym(handle_, "planet_mod_fin"));
        if (auto err = dlerror())
            throw exception_errno(ELIBACC, str(format("%1%: ") % err).c_str());
        new_instance_   = reinterpret_cast<mod_new_instance_t>(dlsym(handle_, "planet_mod_new_instance"));
        if (auto err = dlerror())
            throw exception_errno(ELIBACC, str(format("%1%: ") % err).c_str());
        open_           = reinterpret_cast<mod_open_t>(dlsym(handle_, "planet_mod_open"));
        if (auto err = dlerror())
            throw exception_errno(ELIBACC, str(format("%1%: ") % err).c_str());
        read_           = reinterpret_cast<mod_read_t>(dlsym(handle_, "planet_mod_read"));
        if (auto err = dlerror())
            throw exception_errno(ELIBACC, str(format("%1%: ") % err).c_str());
        write_          = reinterpret_cast<mod_write_t>(dlsym(handle_, "planet_mod_write"));
        if (auto err = dlerror())
            throw exception_errno(ELIBACC, str(format("%1%: ") % err).c_str());
        release_        = reinterpret_cast<mod_release_t>(dlsym(handle_, "planet_mod_release"));
        if (auto err = dlerror())
            throw exception_errno(ELIBACC, str(format("%1%: ") % err).c_str());
        mknod_          = reinterpret_cast<mod_mknod_t>(dlsym(handle_, "planet_mod_mknod"));
        if (auto err = dlerror())
            throw exception_errno(ELIBACC, str(format("%1%: ") % err).c_str());
        rmnod_          = reinterpret_cast<mod_rmnod_t>(dlsym(handle_, "planet_mod_rmnod"));
        if (auto err = dlerror())
            throw exception_errno(ELIBACC, str(format("%1%: ") % err).c_str());
        matching_path_  = reinterpret_cast<mod_matching_path_t>(dlsym(handle_, "planet_mod_is_matching_path"));
        if (auto err = dlerror())
            throw exception_errno(ELIBACC, str(format("%1%: ") % err).c_str());
    }


}   // namespace planet
