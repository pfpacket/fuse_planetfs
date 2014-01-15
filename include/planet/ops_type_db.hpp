#ifndef PLANET_OP_TYPE_DB_HPP
#define PLANET_OP_TYPE_DB_HPP

#include <planet/common.hpp>
#include <vector>
#include <algorithm>
#include <planet/fs_entry.hpp>
#include <planet/fs_ops_type.hpp>
#include <tuple>
#include <boost/container/stable_vector.hpp>

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

        ops_type_db();

        ~ops_type_db();

        void register_ops(priority p, shared_ptr<fs_ops_type> ops);

        void unregister_ops(string_type const& ops_name);

        string_type get_name_by_path(path_type const& path, file_type ft);

        shared_ptr<fs_ops_type> get_ops(string_type const& name);

        bool is_registered(string_type const& name);

        std::vector<info_type> info() const;

        void clear();

    private:
        typedef std::tuple<shared_ptr<fs_ops_type>, priority> ops_info_t;
        typedef boost::container::stable_vector<ops_info_t> ops_info_db;
        enum info_index {
            ops = 0, prio
        };
        ops_info_db ops_types_;
        shared_ptr<core_file_system> fs_root_;

        ops_info_db::iterator get_info_by_name(string_type const& name);

        void remove_ops(string_type const& ops_name);
    };


}   // namespace planet

#endif  // PLANET_OP_TYPE_DB_HPP
