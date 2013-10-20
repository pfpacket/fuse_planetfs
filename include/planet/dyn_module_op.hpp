#ifndef PLANET_DYN_MODULE_OP_HPP
#define PLANET_DYN_MODULE_OP_HPP

#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/dl_loader.hpp>

namespace planet {


    class dyn_module_op : public fs_operation {
    private:
        typedef void (*mod_init_t)(shared_ptr<core_file_system>);
        typedef void (*mod_fin_t)(void);
        typedef shared_ptr<fs_operation> (*mod_new_instance_t)();
        typedef int (*mod_open_t)(shared_ptr<fs_entry>, path_type const&);
        typedef int (*mod_read_t)(shared_ptr<fs_entry>, char *, size_t, off_t);
        typedef int (*mod_write_t)(shared_ptr<fs_entry>, char const *buf, size_t, off_t);
        typedef int (*mod_release_t)(shared_ptr<fs_entry>);
        typedef int (*mod_mknod_t)(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t);
        typedef int (*mod_rmnod_t)(shared_ptr<fs_entry>, path_type const&);
        typedef bool (*mod_matching_path_t)(path_type const&, file_type);

        string_type mod_name_;
        dl_loader loader_;

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
        dyn_module_op(std::string const& module_path, shared_ptr<core_file_system>);

        ~dyn_module_op();

        shared_ptr<fs_operation> new_instance();

        int open(shared_ptr<fs_entry> file_ent, path_type const& path);

        int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset);

        int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset);

        int release(shared_ptr<fs_entry> file_ent);

        int mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t mode, dev_t dev);

        int rmnod(shared_ptr<fs_entry> file_ent, path_type const& path);

        bool is_matching_path(path_type const& path, file_type type);

        void load_all_functions();
    };


}   // namespace planet

#endif  // PLANET_DYN_MODULE_OP_HPP
