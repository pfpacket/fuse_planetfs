#ifndef PLANET_MODULE_LOADER_HPP
#define PLANET_MODULE_LOADER_HPP

#include <planet/common.hpp>
#include <planet/net/common.hpp>
#include <planet/net/tcp/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {


    class module_loader_op : public entry_op {
    private:
        shared_ptr<core_file_system> fs_root_;
        std::vector<string_type> paths_;

        std::once_flag once_flag_;
        std::vector<ops_type_db::info_type> info_;
        decltype(info_)::iterator it_;

    public:

        module_loader_op(shared_ptr<core_file_system>);
        module_loader_op(shared_ptr<core_file_system>, std::vector<string_type> const&);
        ~module_loader_op() = default;

        int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
        int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
        int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
        int release(shared_ptr<fs_entry> file_ent) override;
    };

    class module_loader : public file_ops_type {
    private:
        string_type cwd_;

    public:
        module_loader()
            : file_ops_type("planet.module_loader")
        {
            char cwd[PATH_MAX];
            ::getcwd(cwd, sizeof cwd);
            cwd_ = cwd;
        }

        int install(shared_ptr<core_file_system> fs_root) override;
        int uninstall(shared_ptr<core_file_system> fs_root) override;

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override;

        //int mknod(shared_ptr<fs_entry> file_ent, path_type const& path, mode_t, dev_t) override;
        //int rmnod(shared_ptr<fs_entry> file_ent, path_type const& path) override;

        bool match_path(path_type const& path, file_type type) override;
    };


}   // namespace planet

#endif  // PLANET_MODULE_LOADER_HPP
