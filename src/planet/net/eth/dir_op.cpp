
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/net/eth/dir_op.hpp>
#include <net/if.h>

namespace planet {
namespace net {
namespace eth {


    const string_type dir_type::type_name = "planet.net.eth.dir";

    shared_ptr<entry_op> dir_type::create_op(shared_ptr<core_file_system>)
    {
        return planet::detail::shared_null_ptr;
    }

    int dir_type::install(shared_ptr<core_file_system> fs)
    {
        fs_root_ = fs;
        return 0;
    }

    int dir_type::mknod(shared_ptr<fs_entry>, path_type const& path, mode_t, dev_t)
    {
        auto ifnames = make_unique_ptr(
            if_nameindex(), [](struct if_nameindex *p){ if_freenameindex(p); });
        if (!ifnames)
            throw_system_error(errno);
        for (auto *i = ifnames.get(); !(i->if_index == 0 && i->if_name == NULL); i++)
            fs_root_->mknod(path.string() + "/" + i->if_name, S_IRUSR | S_IWUSR, 0);
        return 0;
    }

    int dir_type::rmnod(shared_ptr<fs_entry>, path_type const&)
    {
        throw_system_error(EPERM);
        return -EPERM;
    }

    bool dir_type::match_path(path_type const& path, file_type type)
    {
        return type == file_type::directory && (path == "/eth" || path == "/ip");
    }


}   // namespace eth
}   // namespace net
}   // namespace planet
