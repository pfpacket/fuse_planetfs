#ifndef PLANET_FS_CORE_HPP
#define PLANET_FS_CORE_HPP

#include <planet/common.hpp>
#include <planet/fs_entry.hpp>
#include <planet/dyn_module_op.hpp>
#include <vector>
#include <algorithm>

namespace planet {


class path_manager {
public:
    typedef unsigned int priority_type;
    typedef op_type_code index_type;
    typedef std::function<bool (path_type const&, file_type)> functor_type;
    typedef std::tuple<priority_type, index_type, functor_type> value_type;
    typedef std::vector<value_type> mapper_type;

    enum priority : priority_type {
        max     = std::numeric_limits<priority_type>::max(),
        normal  = max / 2,
        min     = std::numeric_limits<priority_type>::min(),
    };

    template<typename OpType>
    void add_new_type(priority_type priority)
    {
        add_new_type<OpType>(priority, &OpType::is_matching_path);
    }

    template<typename OpType, typename Callable>
    void add_new_type(priority_type priority, Callable matcher)
    {
        path2type_.push_back(
            std::make_tuple(
                priority, index_type(typeid(OpType)), matcher
            )
        );
        std::stable_sort(path2type_.begin(), path2type_.end(),
            [](value_type const& l, value_type const& r) {
                return std::get<0>(l) > std::get<0>(r);
            }
        );
    }

    template<typename OpType>
    void remove_type()
    {
        path2type_.erase(
            std::remove_if(
                path2type_.begin(), path2type_.end(),
                [](value_type const& t) {
                    return std::get<1>(t) == index_type(typeid(OpType));
                }
            ), path2type_.end()
        );
    }

    index_type matching_type(path_type const& path, file_type type) const
    {
        auto it = std::find_if(path2type_.begin(), path2type_.end(),
            [&](value_type const& t) {
                return std::get<2>(t)(path, type);
            });
        if (it == path2type_.end())
            throw std::runtime_error("No matching type found for " + path.string());
        return std::get<1>(*it);
    }

private:
    mapper_type path2type_;
};

class operation_manager {
public:
    typedef op_type_code index_type;
    typedef shared_ptr<fs_operation> op_type;
    typedef std::map<index_type, op_type> map_type;

    template<typename OpType>
    void add_new_op(shared_ptr<OpType> new_op)
    {
        type2op_.insert(std::make_pair(index_type(typeid(OpType)), new_op));
    }

    template<typename OpType, typename ...Types>
    void add_new_op(Types&& ...args)
    {
        type2op_.insert(
            std::make_pair(index_type(typeid(OpType)),
                std::make_shared<OpType>(std::forward<Types>(args)...)
            )
        );
    }

    template<typename OpType>
    void remove_op()
    {
        auto it = type2op_.begin(), endit = type2op_.end();
        for(; it != endit; ) {
            if (it->first == index_type(typeid(OpType)))
                type2op_.erase(it++);
            else
                ++it;
        }
    }

    op_type matching_op(index_type const& typeindex) const
    {
        return type2op_.at(typeindex);
    }

    op_type operator[](index_type const& typeindex) const
    {
        return matching_op(typeindex);
    }

private:
    map_type type2op_;
};

class core_file_system {
public:

    typedef path_manager::priority priority;

    explicit core_file_system(mode_t);

    ~core_file_system() = default;

    int getattr(path_type const& path, struct stat& stbuf) const;

    int mknod(path_type const& path, mode_t mode, dev_t device);
    int mknod(path_type const& path, mode_t, dev_t, op_type_code);

    int unlink(path_type const& path);

    int mkdir(path_type const& path, mode_t mode);
    int mkdir(path_type const& path, mode_t mode, op_type_code);

    int rmdir(path_type const& path);

    std::vector<std::string> readdir(path_type const& path) const;

    handle_t open(path_type const& path);

    shared_ptr<fs_entry> get_entry_of(path_type const& path) const;

    template<typename OperationType, typename ...Types>
    void install_op(priority p, Types&& ...args)
    {
        path_mgr_.add_new_type<OperationType>(p);
        ops_mgr_.add_new_op<OperationType>(std::forward<Types>(args)...);
    }

    template<typename OperationType>
    void uninstall_op()
    {
        path_mgr_.remove_type<OperationType>();
        ops_mgr_.remove_op<OperationType>();
    }

    void install_dynamic_module(string_type const& mod_name)
    {
        using namespace std::placeholders;
        auto new_op = std::make_shared<dyn_module_op>(mod_name, *this);
        auto functor = std::bind(&dyn_module_op::is_matching_path, new_op.get(), _1, _2);
        path_mgr_.add_new_type<dyn_module_op>(priority::normal, functor);
        ops_mgr_.add_new_op<dyn_module_op>(new_op);
    }

    void uninstall_module(string_type const& mod_name)
    {
        uninstall_op<dyn_module_op>();
    }

private:
    path_manager        path_mgr_;
    operation_manager   ops_mgr_;
    shared_ptr<dentry>  root = detail::shared_null_ptr;

    shared_ptr<fs_entry> get_entry_of__(shared_ptr<dentry> root, path_type const& path) const;
};


}   // namespace planet

#endif  // PLANET_FS_CORE_HPP
