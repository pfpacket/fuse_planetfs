
#include <planet/common.hpp>
#include <planet/fs_core.hpp>
#include <planet/basic_operation.hpp>
#include <planet/operation_layer.hpp>
#include <planet/utils.hpp>
#include <planet/module_loader/module_loader.hpp>
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
                std::get<0>(*it_) + " " + lexical_cast<string_type>(std::get<1>(*it_));
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
        xpv::smatch m;
        int ret = size;
        if (xpv::regex_search(std::string(buf, size), m, xpv::sregex::compile(R"(^load (\w*))"))) {
            typedef core_file_system::priority priority;
            fs_root_->install_module(priority::normal, m[1], paths_);
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
    int module_loader::install(shared_ptr<core_file_system> fs_root)
    {
        fs_root->mknod("/module", 0666);
        return 0;
    }

    int module_loader::uninstall(shared_ptr<core_file_system> fs_root)
    {
        fs_root->unlink("/module");
        return 0;
    }

    shared_ptr<entry_op> module_loader::create_op(shared_ptr<core_file_system> fs_root)
    {
        return make_shared<module_loader_op>(fs_root, std::vector<string_type>{cwd_});
    }

    bool module_loader::match_path(path_type const& path, file_type type)
    {
        return path == "/module" && type == file_type::regular_file;
    }


}   // namespace planet
