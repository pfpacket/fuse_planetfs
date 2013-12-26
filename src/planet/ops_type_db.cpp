
#include <planet/common.hpp>
#include <planet/ops_type_db.hpp>

namespace planet {

    typedef ops_type_db::priority priority;
    typedef ops_type_db::info_type info_type;

    ops_type_db::ops_type_db(shared_ptr<core_file_system> fs_root)
        :   fs_root_(fs_root)
    {
    }

    ops_type_db::~ops_type_db()
    {
        try {
            clear();
        } catch (...) {
            // dtor must not throw any exception
        }
    }

    void ops_type_db::register_ops(priority p, shared_ptr<fs_ops_type> ops)
    {
        try {
            ops_types_.push_back(std::make_tuple(ops, p));
            std::stable_sort(ops_types_.begin(), ops_types_.end(),
                [](ops_type_db::ops_info_t const& l, ops_type_db::ops_info_t const& r) {
                    return std::get<info_index::prio>(l)
                            > std::get<info_index::prio>(r);
                }
            );
            ops->install(fs_root_);
        } catch (...) {
            this->remove_ops(ops->name());
            throw;
        }
    }

    void ops_type_db::unregister_ops(string_type const& ops_name)
    {
        for (auto&& i : ops_types_)
            if (std::get<info_index::ops>(i)->name() == ops_name)
                std::get<info_index::ops>(i)->uninstall(this->fs_root_);
        this->remove_ops(ops_name);
    }

    string_type ops_type_db::get_name_by_path(path_type const& path, file_type ft)
    {
        auto it = std::find_if(ops_types_.begin(), ops_types_.end(),
            [&](ops_type_db::ops_info_t const& info) {
                return std::get<info_index::ops>(info)->match_path(path, ft);
            });
        if (it == ops_types_.end())
            throw std::runtime_error("No matching operaion type found for " + path.string());
        return std::get<info_index::ops>(*it)->name();
    }

    shared_ptr<fs_ops_type> ops_type_db::get_ops(string_type const& name)
    {
        auto it = this->get_info_by_name(name);
        if (it == ops_types_.end())
            throw std::runtime_error("get_ops: No such ops: " + name);
        return std::get<info_index::ops>(*it);
    }

    bool ops_type_db::is_registered(string_type const& name)
    {
        return (this->get_info_by_name(name) != ops_types_.end());
    }

    std::vector<info_type> ops_type_db::info() const
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

    void ops_type_db::clear()
    {
        for (auto&& i : ops_types_)
            std::get<info_index::ops>(i)->uninstall(fs_root_);
        ops_types_.clear();
    }

    ops_type_db::ops_info_db::iterator ops_type_db::get_info_by_name(string_type const& name)
    {
        return std::find_if(ops_types_.begin(), ops_types_.end(),
            [&name](ops_type_db::ops_info_t const& info) {
                return std::get<info_index::ops>(info)->name() == name;
            });
    }

    void ops_type_db::remove_ops(string_type const& ops_name)
    {
        auto it = std::remove_if(
            ops_types_.begin(), ops_types_.end(),
            [&ops_name](ops_type_db::ops_info_t const& i) {
                return std::get<info_index::ops>(i)->name() == ops_name;
            });
        if (it != ops_types_.end())
            ops_types_.erase(it, ops_types_.end());
    }


}   // namespace planet
