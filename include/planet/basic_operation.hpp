#ifndef PLANET_BASIC_OPERATION_HPP
#define PLANET_BASIC_OPERATION_HPP

#include <planet/common.hpp>

namespace planet {


    class fs_entry;
    class file_entry;
    class core_file_system;

    //
    // planet core handler
    //
    class entry_op {
    public:
        entry_op() = default;

        virtual ~entry_op() noexcept
        {
        }

        // Open, read, write and close hook functions
        virtual int open(shared_ptr<fs_entry>, path_type const&)
        {
            return -EPERM;
        }

        virtual int read(shared_ptr<fs_entry>, char *, size_t, off_t)
        {
            return -EPERM;
        }

        virtual int write(shared_ptr<fs_entry>, char const *, size_t, off_t)
        {
            return -EPERM;
        }

        virtual int release(shared_ptr<fs_entry>)
        {
            return -EPERM;
        }

        virtual int poll(pollmask_t&)
        {
            return -EPERM;
        }
    };

    //
    // Note: All of planetfs operations must inherit entry_op
    //

    // default file operation which is used if no other operations match the target path
    class default_file_op : public entry_op {
    public:
        default_file_op() = default;
        default_file_op(shared_ptr<core_file_system>)
        {
        }

        virtual ~default_file_op() = default;

        virtual int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
        virtual int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
        virtual int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
        virtual int release(shared_ptr<fs_entry> file_ent) override;
    };

    // default dir operation which is used if no other operations match the target path
    class default_dir_op : public entry_op {
    public:
        default_dir_op() = default;
        default_dir_op(shared_ptr<core_file_system>)
        {
        }

        virtual ~default_dir_op() = default;

        virtual int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
        virtual int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
        virtual int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
        virtual int release(shared_ptr<fs_entry> file_ent) override;
    };


}   //namespace planet

#endif  // PLANET_BASIC_OPERATION_HPP
