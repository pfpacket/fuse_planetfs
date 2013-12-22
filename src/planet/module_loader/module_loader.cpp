
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
#include <planet/utils.hpp>
#include <planet/module_loader/module_loader.hpp>
#include <planet/request_parser.hpp>
#include <mutex>

namespace planet {


    //
    // module_loader_op
    //
    module_loader_op::module_loader_op(shared_ptr<core_file_system> fs_root)
        :   fs_root_(fs_root)
    {
    }

    module_loader_op::module_loader_op(
        shared_ptr<core_file_system> fs_root, std::vector<string_type> const& paths
    )
        :   fs_root_(fs_root), paths_(paths)
    {
    }

    int module_loader_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        return 0;
    }

    int module_loader_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        int ret = 0;
        std::call_once(once_flag_, [this](){
            this->info_ = fs_root_->get_ops_db().info();
            this->it_   = info_.begin();
        });
        if (it_ != info_.end()) {
            string_type line =
                std::get<0>(*it_) + '\t' + lexical_cast<string_type>(std::get<1>(*it_));
            ret = (line.length() + 2 > size) ? size : line.length() + 2;
            std::copy_n(line.begin(), ret - 2, buf);
            buf[ret - 2] = '\n';
            buf[ret - 1] = '\0';
            ++it_;
        }
        return ret;
    }

    int module_loader_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        int ret = size;
        request_parser parser;
        if (parser.parse(string_type(buf, size))) {
            typedef core_file_system::priority priority;
            if (parser.get_command() == "load")
                for (auto&& mod_name : parser.get_args())
                    fs_root_->install_module(priority::normal, mod_name[0], paths_);
            else if (parser.get_command() == "unload")
                for (auto&& mod_name : parser.get_args())
                    fs_root_->uninstall_module(mod_name[0]);
        } else
            ret = -ENOTSUP;
        return ret;
    }

    int module_loader_op::release(shared_ptr<fs_entry> file_ent)
    {
        return 0;
    }

    //
    // module_loader
    //
    string_type const module_loader::file_path = "/modules";

    module_loader::module_loader()
        : file_ops_type("planet.module_loader")
    {
        char cwd[PATH_MAX];
        if (!::getcwd(cwd, sizeof cwd))
            throw_system_error(errno, "getting cwd");
        cwd_ = cwd;
    }

    int module_loader::install(shared_ptr<core_file_system> fs_root)
    {
        fs_root->mknod(module_loader::file_path, 0666);
        return 0;
    }

    int module_loader::uninstall(shared_ptr<core_file_system> fs_root)
    {
        fs_root->unlink(module_loader::file_path);
        return 0;
    }

    shared_ptr<entry_op> module_loader::create_op(shared_ptr<core_file_system> fs_root)
    {
        return make_shared<module_loader_op>(fs_root, std::vector<string_type>{cwd_});
    }

    bool module_loader::match_path(path_type const& path, file_type type)
    {
        return path == module_loader::file_path && type == file_type::regular_file;
    }


}   // namespace planet
