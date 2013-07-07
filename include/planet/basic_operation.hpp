#ifndef PLANET_BASIC_OPERATION_HPP
#define PLANET_BASIC_OPERATION_HPP

#include <planet/common.hpp>


namespace planet {

class fs_entry;
class file_entry;

//
// planet core handler
//
class fs_operation {
protected:
    static std::vector<char>& data_vector(file_entry& file);
public:
    virtual ~fs_operation() noexcept
    {
    }

    // Create new instance of this file operation
    virtual shared_ptr<fs_operation> new_instance() const
    {
        return detail::shared_null_ptr;
    }

    // Open, read, write and close hook functions
    virtual int open(shared_ptr<fs_entry>, path_type const&)
    {
        return 0;
    }

    virtual int read(shared_ptr<fs_entry>, char *buf, size_t size, off_t offset)
    {
        return 0;
    }

    virtual int write(shared_ptr<fs_entry>, char const *buf, size_t size, off_t offset)
    {
        return 0;
    }

    virtual int release(shared_ptr<fs_entry>)
    {
        return 0;
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
};


//
// Note: All of planetfs operations must inherit fs_operation
//       and implement static function named `is_matching_path()`
//       which returns true if the given path is for the operation
//

// default file operation which is used if no other operations match the target path
class default_file_op final : public fs_operation {
public:
    ~default_file_op() = default;

    shared_ptr<fs_operation> new_instance() const override;
    int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
    int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<fs_entry> file_ent) override;
    int mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t) override;
    int rmnod(shared_ptr<fs_entry>, path_type const&) override;
    static bool is_matching_path(path_type const&, file_type);
};

// default dir operation which is used if no other operations match the target path
class default_dir_op final : public fs_operation {
public:
    default_dir_op()
    {
        ::syslog(LOG_NOTICE, "%s: ctor called", __PRETTY_FUNCTION__);
    }
    //~default_dir_op() = default;
    ~default_dir_op()
    {
        ::syslog(LOG_NOTICE, "%s: dtor called", __PRETTY_FUNCTION__);
    }

    shared_ptr<fs_operation> new_instance() const override;
    int open(shared_ptr<fs_entry> file_ent, path_type const& path) override;
    int read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<fs_entry> file_ent) override;
    int mknod(shared_ptr<fs_entry>, path_type const&, mode_t, dev_t) override;
    int rmnod(shared_ptr<fs_entry>, path_type const&) override;
    static bool is_matching_path(path_type const&, file_type);
};


}   //namespace planet

#endif  // PLANET_BASIC_OPERATION_HPP
