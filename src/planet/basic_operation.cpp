
#include <planet/common.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {


    std::vector<char>& planet_operation::data_vector(file_entry& file)
    {
        return file.data_;
    }

    shared_ptr<planet_operation> default_file_op::new_instance() const
    {
        return std::make_shared<default_file_op>();
    }

    int default_file_op::open(shared_ptr<file_entry> file_ent, path_type const& path)
    {
        return 0;
    }

    int default_file_op::read(shared_ptr<file_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        if (file_ent->size() < size + offset)
            size = file_ent->size() - offset;
        std::copy_n(this->data_vector(*file_ent).begin() + offset, size, buf);
        return size;
    }

    int default_file_op::write(shared_ptr<file_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        if (file_ent->size() < size + offset)
            size = file_ent->size() - offset;
        std::copy_n(buf, size, this->data_vector(*file_ent).begin() + offset);
        return size;
    }

    int default_file_op::release(shared_ptr<file_entry> file_ent)
    {
        return 0;
    }


}   // namespace planet
