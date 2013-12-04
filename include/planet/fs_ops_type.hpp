#ifndef PLANET_FILESYSTEM_OPERATIONS_TYPE_HPP
#define PLANET_FILESYSTEM_OPERATIONS_TYPE_HPP

#include <planet/common.hpp>
#include <planet/basic_operation.hpp>

namespace planet {

    class core_file_system;

    //
    // fs_ops_type
    //  Represents file operation type
    //  Registering new operations requires this class
    //
    class fs_ops_type {
    private:
        string_type op_name_;

    public:
        fs_ops_type(string_type name)
            :   op_name_(std::move(name))
        {
        }

        virtual ~fs_ops_type() = default;

        string_type const& name() const
        {
            return op_name_;
        }

        // This function is called when installing the type of operation
        virtual int install(shared_ptr<core_file_system>)
        {
            return 0;
        }

        // This function is called when installing the type of operation
        virtual int uninstall(shared_ptr<core_file_system>)
        {
            return 0;
        }

        // Create new instance of entry operation
        virtual shared_ptr<entry_op> create_op(shared_ptr<core_file_system>)
        {
            return detail::shared_null_ptr;
        }

        // Initialize the first arguemnt of fs_entry
        // shared_ptr<fs_entry> is the new file entry which a new file operation will use
        virtual int mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t)
        {
            return 0;
        }

        // Destoroy the first argument of fs_entry
        // shared_ptr<fs_entry> was used by an other file operation, and now no one never uses it
        virtual int rmnod(shared_ptr<fs_entry>, path_type const&)
        {
            return 0;
        }

        // Return true if the given path is for the operation
        virtual bool match_path(path_type const&, file_type)
        {
            return false;
        }
    };

    class file_ops_type : public fs_ops_type {
    public:
        typedef default_file_op op_type;

        file_ops_type() : fs_ops_type("planet.file_ops_type")
        {
        }

        file_ops_type(string_type name) : fs_ops_type(name)
        {
        }

        virtual ~file_ops_type() = default;

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override
        {
            return make_shared<op_type>(fs_root);
        }

        bool match_path(path_type const&, file_type type) override
        {
            return type == file_type::regular_file;
        }
    };

    class dir_ops_type : public fs_ops_type {
    public:
        typedef default_dir_op op_type;

        dir_ops_type() : fs_ops_type("planet.dir_ops_type")
        {
        }

        dir_ops_type(string_type name) : fs_ops_type(name)
        {
        }

        virtual ~dir_ops_type() = default;

        shared_ptr<entry_op> create_op(shared_ptr<core_file_system> fs_root) override
        {
            return make_shared<op_type>(fs_root);
        }

        int mknod(shared_ptr<fs_entry>, path_type const& path, mode_t, dev_t) override
        {
            return 0;
        }

        int rmnod(shared_ptr<fs_entry>, path_type const&) override
        {
            return 0;
        }

        bool match_path(path_type const&, file_type type) override
        {
            return type == file_type::directory;
        }
    };


}   // namespace planet

#endif  // PLANET_FILESYSTEM_OPERATIONS_TYPE_HPP
