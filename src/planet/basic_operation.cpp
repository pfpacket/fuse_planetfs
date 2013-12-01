
#include <planet/common.hpp>
#include <planet/utils.hpp>
#include <planet/basic_operation.hpp>
#include <planet/fs_core.hpp>

namespace planet {


    //
    // default_file_op
    //
    int default_file_op::open(shared_ptr<fs_entry> file_ent, path_type const& path)
    {
        return 0;
    }

    int default_file_op::read(shared_ptr<fs_entry> file_ent, char *buf, size_t size, off_t offset)
    {
        if (file_ent->size() < size + offset)
            size = file_ent->size() - offset;
        std::copy_n(file_cast(file_ent)->data().begin() + offset, size, buf);
        return size;
    }

    int default_file_op::write(shared_ptr<fs_entry> file_ent, char const *buf, size_t size, off_t offset)
    {
        auto fentry = file_cast(file_ent);
        if (fentry->size() < size + offset)
            fentry->data().resize(size + offset, 0);
        std::copy_n(buf, size, fentry->data().begin() + offset);
        return size;
    }

    int default_file_op::release(shared_ptr<fs_entry> file_ent)
    {
        return 0;
    }

    //
    // default_dir_op
    //
    int default_dir_op::open(shared_ptr<fs_entry> dir_ent, path_type const& path)
    {
        return 0;
    }

    int default_dir_op::read(shared_ptr<fs_entry> dir_ent, char *buf, size_t size, off_t offset)
    {
        return 0;
    }

    int default_dir_op::write(shared_ptr<fs_entry> dir_ent, char const *buf, size_t size, off_t offset)
    {
        return 0;
    }

    int default_dir_op::release(shared_ptr<fs_entry> dir_ent)
    {
        return 0;
    }


}   // namespace planet
