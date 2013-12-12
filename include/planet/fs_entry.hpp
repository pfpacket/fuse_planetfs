#ifndef PLANET_FS_ENTRY_HPP
#define PLANET_FS_ENTRY_HPP

#include <planet/common.hpp>
#include <vector>
#include <chrono>
#include <sys/stat.h>
#include <planet/handle.hpp>

namespace planet {


    // inode structure
    class st_inode {
    public:
        dev_t dev = 0;
        mode_t mode = 0;
        uid_t uid = 0;
        gid_t gid = 0;
        std::chrono::system_clock::time_point
            atime = std::chrono::system_clock::now(),
            mtime = std::chrono::system_clock::now(),
            ctime = std::chrono::system_clock::now();

        decltype(atime) const& last_access_time() const;

        decltype(mtime) const& last_modified_time() const;

        decltype(ctime) const& last_stat_changed_time() const;

        template<typename Clock, typename Duration>
        static std::time_t to_time_t(std::chrono::time_point<Clock, Duration> const& t)
        {
            return Clock::to_time_t(t);
        }
    };


    //
    // fs_entry abstract base class
    //    for every filesystem entry
    //
    class fs_entry {
    public:
        virtual ~fs_entry() = default;

        // Entry name not a path
        virtual string_type const& name() const = 0;

        // Entry type code
        virtual file_type type() const noexcept = 0;

        // The size of this entry
        virtual size_t size() const noexcept = 0;

        // Inode of this entry
        virtual st_inode const& inode() const = 0;
        virtual void inode(st_inode const&) = 0;

        // Operations name
        virtual string_type const& ops_name() const = 0;
    };

    class file_entry : public fs_entry {
        typedef char value_type;
        typedef std::vector<value_type> buffer_type;

        std::string name_, ops_name_;
        st_inode inode_;
        std::vector<value_type> data_;

    public:
        file_entry(string_type name, string_type ops_name, st_inode const& sti);

        virtual ~file_entry() = default;

        string_type const& name() const override;

        file_type type() const noexcept override;

        size_t size() const noexcept override;

        st_inode const& inode() const override;

        void inode(st_inode const& inode) override;

        string_type const& ops_name() const override;

        decltype(data_)& data();

        decltype(data_) const& data() const;

        char const *get_data_ptr() const;
    };

    class dentry : public fs_entry {
    public:
        typedef shared_ptr<fs_entry> entry_type;
    private:
        enum {default_vector_size = 512};

        string_type name_, ops_name_;
        st_inode inode_;
        std::vector<entry_type> entries_;

    public:
        dentry(string_type name, string_type ops_name, st_inode const& sti = {});

        virtual ~dentry();

        string_type const& name() const override;

        file_type type() const noexcept override;

        size_t size() const noexcept override;

        st_inode const& inode() const override;

        void inode(st_inode const& inode) override;

        string_type const& ops_name() const override;

        decltype(entries_) const& entries() const;

        entry_type search_entries(string_type const& name) const;

        template<typename EntryType, typename... Types>
        shared_ptr<EntryType> add_entry(Types&&... args)
        {
            auto entry = std::make_shared<EntryType>(std::forward<Types>(args)...);
            entries_.push_back(entry);
            return entry;
        }

        bool remove_entry(string_type const& name);
    };


}   // namespace planet

#endif  // PLANET_FS_ENTRY_HPP
