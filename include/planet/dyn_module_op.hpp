#ifndef PLANET_DYN_MODULE_OP_HPP
#define PLANET_DYN_MODULE_OP_HPP

#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <dlfcn.h>

namespace planet {


    class fs_entry;
    class fs_operation;
    class core_file_system;
    class fs_operation;

    class dyn_module_op : public fs_operation {
    private:
        typedef void (*mod_init_t)(core_file_system *);
        typedef void (*mod_fin_t)(void);
        typedef shared_ptr<fs_operation> (*mod_new_instance_t)();
        typedef int (*mod_open_t)(shared_ptr<fs_entry>, path_type const&);
        typedef int (*mod_read_t)(shared_ptr<fs_entry>, char *, size_t, off_t);
        typedef int (*mod_write_t)(shared_ptr<fs_entry>, char const *buf, size_t, off_t);
        typedef int (*mod_release_t)(shared_ptr<fs_entry>);
        typedef int (*mod_mknod_t)(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t);
        typedef int (*mod_rmnod_t)(shared_ptr<fs_entry>, path_type const&);
        typedef bool (*mod_matching_path_t)(path_type const&, file_type);

        std::string mod_name_;
        raii_wrapper dl_remover_;
        void *handle_;

        // loaded module's functions
        mod_init_t          init_;
        mod_fin_t           fin_;
        mod_new_instance_t  new_instance_;
        mod_open_t          open_;
        mod_read_t          read_;
        mod_write_t         write_;
        mod_release_t       release_;
        mod_mknod_t         mknod_;
        mod_rmnod_t         rmnod_;
        mod_matching_path_t matching_path_;

    public:
        dyn_module_op(std::string const& module_path, core_file_system& fs_root)
            :   mod_name_(module_path),
                handle_(dlopen(module_path.c_str(), RTLD_LAZY))
        {
            if (!handle_)
                throw std::runtime_error(dlerror());
            dl_remover_ = raii_wrapper([this](){ dlclose(handle_); });
            reload();
            init_(&fs_root);
        }

        ~dyn_module_op()
        {
            try {
                fin_();
            } catch (...) {
                // dtor must not throw any exceptions
            }
        }

        shared_ptr<fs_operation> new_instance()
        {
            return new_instance_();
        }

        int open(shared_ptr<fs_entry> file_ent, path_type const& path)
        {
            return open_(file_ent, path);
        }

        int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
        {
            return read_(file_ent, buf, size, offset);
        }

        int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
        {
            return write_(file_ent, buf, size, offset);
        }

        int release(shared_ptr<fs_entry> file_ent)
        {
            return release_(file_ent);
        }

        int mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t mode, dev_t dev)
        {
            return mknod_(file_ent, path, mode, dev);
        }

        int rmnod(shared_ptr<fs_entry> file_ent, path_type const& path)
        {
            return rmnod_(file_ent, path);
        }

        bool is_matching_path(path_type const& path, file_type type)
        {
            return matching_path_(path, type);
        }

        void reload()
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
    };


}   // namespace planet

#endif  // PLANET_DYN_MODULE_OP_HPP
