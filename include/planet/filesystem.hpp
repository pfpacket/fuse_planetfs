#ifndef PLANET_FILESYSTEM_HPP
#define PLANET_FILESYSTEM_HPP

#ifndef _FILE_OFFSET_BITS
#   define _FILE_OFFSET_BITS 64
#endif

extern "C" {
#include <ixp.h>
}
#include <planet/common.hpp>
#include <planet/fs_core.hpp>

namespace planet {

    struct ixp_context {
        int fd;
        IxpServer server;
        IxpConn *conn;
    };

    class filesystem {
    private:
        shared_ptr<ops_type_db>         ops_db_;
        shared_ptr<core_file_system>    root_;
        ixp_context ctx_;

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

        void listen(int argc, char **argv, Ixp9Srv const& srv, void *data)
        {
            ctx_.fd = ixp_announce(argv[1]);
            if (ctx_.fd < 0)
                throw std::runtime_error(std::string("announce: ") + ixp_errbuf());

            ctx_.conn = ixp_listen(&ctx_.server, ctx_.fd, (void *)&srv, &ixp_serve9conn, nullptr);
            if (!ctx_.conn)
                throw std::runtime_error(std::string("listen: ") + ixp_errbuf());
        }

        int start_main()
        {
            return ixp_serverloop(&ctx_.server);
        }

        ixp_context& context()
        {
            return ctx_;
        }

        shared_ptr<core_file_system> root()
        {
            return root_;
        }
    };


}   // namespace planet

#endif  // PLANET_FILESYSTEM_HPP
