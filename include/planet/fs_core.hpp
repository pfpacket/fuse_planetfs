#ifndef PLANET_FS_CORE_HPP
#define PLANET_FS_CORE_HPP

#include <planet/common.hpp>
#include <vector>
#include <algorithm>
#include <planet/fs_entry.hpp>
#include <planet/fs_ops_type.hpp>
#include <tuple>

namespace planet {

    class core_file_system;

    class ops_type_db {
    public:
        typedef unsigned int priority_type;
        enum priority : priority_type {
            high    = std::numeric_limits<priority_type>::max(),
            normal  = high / 2,
            low     = std::numeric_limits<priority_type>::min() + 1,
        };

        typedef std::tuple<string_type, priority> info_type;

        ops_type_db(shared_ptr<core_file_system> fs_root)
            :   fs_root_(fs_root)
        {
        }

        ~ops_type_db()
        {
            try {
                clear();
            } catch (...) {
                // dtor must not throw any exception
            }
        }

        void register_ops(priority p, shared_ptr<fs_ops_type> ops)
        {
            ops_types_.push_back(std::make_tuple(ops, p));
            std::stable_sort(ops_types_.begin(), ops_types_.end(),
                [](ops_info_t const& l, ops_info_t const& r) {
                    return std::get<info_index::prio>(l)
                            > std::get<info_index::prio>(r);
                }
            );
            ops->install(fs_root_);
        }

        void unregister_type(string_type const& ops_name)
        {
            syslog(LOG_NOTICE, "register_ops: ops_name=%s", ops_name.c_str());
            for (auto&& i : ops_types_)
                if (std::get<info_index::ops>(i)->name() == ops_name)
                    std::get<info_index::ops>(i)->uninstall(this->fs_root_);
            auto it = std::remove_if(
                ops_types_.begin(), ops_types_.end(),
                [&ops_name](ops_info_t const& t) {
                    return std::get<info_index::ops>(t)->name() == ops_name;
                });
            if (it != ops_types_.end())
                ops_types_.erase(it, ops_types_.end());
        }

        // TODO: Why do I receive file_type? fs_entry should be received, not file_type
        string_type get_name_by_path(path_type const& path, file_type ft)
        {
            auto it = std::find_if(ops_types_.begin(), ops_types_.end(),
                [&](ops_info_t const& info) {
                    return std::get<info_index::ops>(info)->match_path(path, ft);
                });
            if (it == ops_types_.end())
                throw std::runtime_error("No matching operaion type found for " + path.string());
            return std::get<info_index::ops>(*it)->name();
        }

        shared_ptr<fs_ops_type> get_ops(string_type name)
        {
            return std::get<info_index::ops>(*this->get_info_by_name(name));
        }

        shared_ptr<entry_op> get_entry_op(string_type name)
        {
            auto it = this->get_info_by_name(std::move(name));
            return (it == ops_types_.end()) ? detail::shared_null_ptr
                : std::get<info_index::ops>(*it)->create_op(fs_root_);
        }

        bool is_registered(string_type const& name)
        {
            return (this->get_info_by_name(name) != ops_types_.end());
        }

        std::vector<info_type> info() const
        {
            std::vector<info_type> result;
            for (auto&& i : ops_types_)
                result.push_back(
                    std::make_tuple(
                        std::get<info_index::ops>(i)->name(),
                        std::get<info_index::prio>(i)
                    )
                );
            return result;
        }

        void clear()
        {
            for (auto&& i : ops_types_)
                std::get<info_index::ops>(i)->uninstall(fs_root_);
            ops_types_.clear();
        }

    private:
        typedef std::tuple<shared_ptr<fs_ops_type>, priority> ops_info_t;
        typedef std::vector<ops_info_t> ops_info_db;
        enum info_index {
            ops = 0, prio
        };
        ops_info_db ops_types_;
        shared_ptr<core_file_system> fs_root_;

        ops_info_db::iterator get_info_by_name(string_type const& name)
        {
            return std::find_if(ops_types_.begin(), ops_types_.end(),
                [&name](ops_info_t const& info) {
                    return std::get<info_index::ops>(info)->name() == name;
                });
        }
    };


    class filesystem;
    class core_file_system
        : public std::enable_shared_from_this<core_file_system> {
    public:

        friend class filesystem;
        typedef ops_type_db::priority priority;

        int getattr(path_type const& path, struct stat& stbuf) const;

        int mknod(path_type const& path, mode_t mode, dev_t device);
        int mknod(path_type const& path, mode_t, dev_t, string_type const&);

        int unlink(path_type const& path, bool force = false);

        int mkdir(path_type const& path, mode_t mode);
        int mkdir(path_type const& path, mode_t mode, string_type const&);

        int rmdir(path_type const& path, bool force = false);

        std::vector<std::string> readdir(path_type const& path) const;

        handle_t open(path_type const& path);

        shared_ptr<fs_entry> get_entry_of(path_type const& path) const;

        void install_ops(priority p, shared_ptr<fs_ops_type> ops);

        template<typename OperationType, typename ...Types>
        void install_ops(priority p, Types&& ...args)
        {
            if (auto ops_db = ops_db_.lock())
                ops_db->register_ops(
                    p, make_shared<OperationType>(std::forward<Types>(args)...)
                );
        }

        void uninstall_ops(string_type const& name);

        void install_module(priority, string_type const&);

        void uninstall_module(string_type const&);

    private:
        shared_ptr<dentry>      root    = detail::shared_null_ptr;
        weak_ptr<ops_type_db>   ops_db_;

        core_file_system() = default;
        static shared_ptr<fs_entry> get_entry_of(shared_ptr<dentry> root, path_type const& path);
    };


}   // namespace planet

#endif  // PLANET_FS_CORE_HPP
