#ifndef PLANET_FILESYSTEM_HPP
#define PLANET_FILESYSTEM_HPP

#include <planet/common.hpp>
#include <planet/fs_core.hpp>

namespace planet {


    class filesystem {
    private:
        shared_ptr<ops_type_db>         ops_db_;
        shared_ptr<core_file_system>    root_;

    public:
        typedef ops_type_db::priority priority;

        filesystem(mode_t root_mode)
        {
            root_   = shared_ptr<core_file_system>(new core_file_system());
            ops_db_ = make_shared<ops_type_db>();

            // This is not circular reference of shared_ptr
            root_->ops_db_ = ops_db_;    // root_.ops_db_ is a weak_ptr

            // Create root directory of this filesystem
            st_inode new_inode;
            new_inode.mode = root_mode | S_IFDIR;
            root_->root = std::make_shared<dentry>("/", "dir_ops_type", new_inode);

            // Install default file and directory operation
            root_-> template install_ops<file_ops_type>(priority::low);
            root_-> template install_ops<dir_ops_type>(priority::low);
        }

        ~filesystem()
        {
            root_->uninstal_all();
            // Destroy ops_db_ first because ops_db_ has a reference to root_
            ops_db_->clear();
            ops_db_.reset();
            BOOST_LOG_TRIVIAL(debug) << "filesystem: dtor: core_file_system: use_count=" << root_.use_count();
        }

        shared_ptr<core_file_system> root()
        {
            return root_;
        }
    };


}   // namespace planet

#endif  // PLANET_FILESYSTEM_HPP
