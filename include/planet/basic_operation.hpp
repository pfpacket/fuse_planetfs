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
    virtual shared_ptr<planet_operation> new_instance() const = 0;
    virtual int open(shared_ptr<file_entry>, path_type const&) = 0;
    virtual int read(shared_ptr<file_entry>, char *buf, size_t size, off_t offset) = 0;
    virtual int write(shared_ptr<file_entry>, char const *buf, size_t size, off_t offset) = 0;
    virtual int release(shared_ptr<file_entry>) = 0;
};


//
// Note: All of planetfs operation must inherit planet_operation
//       and implement static function named `is_matching_path()`
//       which returns true if the given path is for the operation
//

class default_file_op final : public planet_operation {
public:
    ~default_file_op() = default;

    shared_ptr<planet_operation> new_instance() const;
    int open(shared_ptr<file_entry> file_ent, path_type const& path) override;
    int read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset) override;
    int write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset) override;
    int release(shared_ptr<file_entry> file_ent) override;
    static bool is_matching_path(path_type const&)
    {
        return true;
    }
};


}   //namespace planet

#endif  // PLANET_BASIC_OPERATION_HPP
