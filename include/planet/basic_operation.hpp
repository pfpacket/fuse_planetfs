#ifndef PLANET_BASIC_OPERATION_HPP
#define PLANET_BASIC_OPERATION_HPP

#include <planet/common.hpp>


namespace planet {

class file_entry;

//
// planet core handler
//
class planet_operation {
protected:
    static std::vector<char>& data_vector(file_entry& file);
public:
    virtual ~planet_operation() noexcept
    {
    }

    // Create new instance of this file operation
    virtual shared_ptr<planet_operation> new_instance() const = 0;

    // Open, read, write and close hook functions
    virtual int open(shared_ptr<file_entry>, path_type const&) = 0;
    virtual int read(shared_ptr<file_entry>, char *buf, size_t size, off_t offset) = 0;
    virtual int write(shared_ptr<file_entry>, char const *buf, size_t size, off_t offset) = 0;
    virtual int release(shared_ptr<file_entry>) = 0;

    // Initialize the first arguemnt of file_entry
    // shared_ptr<file_entry> is the new file entry which a new file operation will use
    virtual int mknod(shared_ptr<file_entry>, path_type const&, mode_t, dev_t) = 0;
    // Destoroy the first arguments of file_entry
    // shared_ptr<file_entry> was used by an other file operation, and now no one never uses it
    virtual int rmnod(shared_ptr<file_entry>, path_type const&) = 0;
};


//
// Note: All of planetfs operations must inherit planet_operation
//       and implement static function named `is_matching_path()`
//       which returns true if the given path is for the operation
//

// default file operation which is used if no other operations match the target path
class default_file_op final : public planet_operation {
public:
    ~default_file_op() = default;

    shared_ptr<planet_operation> new_instance() const;
    int open(shared_ptr<file_entry> file_ent, path_type const& path) override;
    int read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<file_entry> file_ent) override;
    int mknod(shared_ptr<file_entry>, path_type const&, mode_t, dev_t) override;
    int rmnod(shared_ptr<file_entry>, path_type const&) override;
    static bool is_matching_path(path_type const&)
    {
        return true;
    }
};


}   //namespace planet

#endif  // PLANET_BASIC_OPERATION_HPP