#ifndef PLANET_FS_CORE_HPP
#define PLANET_FS_CORE_HPP

#include <planet/common.hpp>
#include <vector>
#include <algorithm>
#include <planet/fs_entry.hpp>

namespace planet {


class path_manager {
public:
    typedef unsigned int priority_type;
    typedef op_type_code index_type;
    typedef std::function<bool (path_type const&, file_type)> functor_type;
    typedef std::tuple<priority_type, index_type, functor_type> value_type;
    typedef std::vector<value_type> mapper_type;

    enum priority : priority_type {
        high    = std::numeric_limits<priority_type>::max(),
        normal  = high / 2,
        low     = std::numeric_limits<priority_type>::min(),
    };

    template<typename OpType>
    void add_new_type(priority_type priority)
    {
        add_new_type<OpType>(priority, index_type::get<OpType>(), &OpType::is_matching_path);
    }

    template<typename OpType, typename Callable>
    void add_new_type(priority_type priority, index_type const& op_code, Callable matcher)
    {
        path2type_.push_back(
            std::make_tuple(
                priority, op_code, matcher
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
        remove_type(index_type::get<OpType>());
    }

    void remove_type(index_type const& op_code)
    {
        auto it = std::remove_if(
            path2type_.begin(), path2type_.end(),
            [&op_code](value_type const& t) {
                return std::get<1>(t) == op_code;
            }
        );
        if (it != path2type_.end())
            path2type_.erase(it, path2type_.end());
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

    void clear()
    {
        path2type_.clear();
    }

private:
    mapper_type path2type_;
};

class operation_manager {
private:
    template<typename Container, typename Pred>
    static void release_op_if(Container& cont, Pred const& predicate)
    {
        for (auto it = cont.begin(); it != cont.end(); ++it)
            if (predicate(*it))
                it->second.reset();
    }

public:
    typedef op_type_code index_type;
    typedef shared_ptr<fs_operation> op_type;
    typedef std::map<index_type, op_type> map_type;
    typedef map_type::value_type value_type;

    template<typename OpType>
    void add_new_op(string_type const& name, shared_ptr<OpType> new_op)
    {
        type2op_.insert(std::make_pair(index_type(name), new_op));
    }

    template<typename OpType, typename ...Types>
    void add_new_op(Types&& ...args)
    {
        type2op_.insert(
            std::make_pair(index_type::get<OpType>(),
                std::make_shared<OpType>(std::forward<Types>(args)...)
            )
        );
    }

    template<typename OpType>
    void remove_op()
    {
        remove_op(index_type::get<OpType>());
    }

    void remove_op(index_type const& op_code)
    {
        release_op_if(
            type2op_,
            [&op_code](value_type const& p) {
                return p.first == op_code;
            }
        );
        type2op_.erase(op_code);
    }

    op_type matching_op(index_type const& typeindex) const
    try {
        return type2op_.at(typeindex);
    } catch (...) {
        throw std::runtime_error(str(
            format("%1%: try to get unknown operation for: %2%")
                % "operation_manager" % typeindex.name()
        ));
    }

    op_type operator[](index_type const& typeindex) const
    {
        return matching_op(typeindex);
    }

    void clear(shared_ptr<core_file_system> fs_root)
    {
        for (auto&& pair : type2op_)
            pair.second->uninstall(fs_root);
        type2op_.clear();
    }

private:
    map_type type2op_;
};

class core_file_system
    : public std::enable_shared_from_this<core_file_system> {
public:

    typedef path_manager::priority priority;

    core_file_system(
        mode_t, weak_ptr<path_manager>, weak_ptr<operation_manager>
    );

    ~core_file_system();

    template<typename ...Types>
    static shared_ptr<core_file_system> create(Types&& ...args)
    {
        auto new_fs =
            std::make_shared<core_file_system>(std::forward<Types>(args)...);
        new_fs-> template install_op<default_file_op>(priority::low);
        new_fs-> template install_op<default_dir_op>(priority::low);
        return new_fs;
    }

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
        auto path_mgr = path_mgr_.lock();
        auto ops_mgr  = ops_mgr_.lock();
        path_mgr->add_new_type<OperationType>(p);
        ops_mgr->add_new_op<OperationType>(
            this->shared_from_this(), std::forward<Types>(args)...
        );
        ops_mgr->matching_op(op_type_code::get<OperationType>())->install(this->shared_from_this());
    }

    template<typename OperationType>
    void uninstall_op()
    {
        auto path_mgr = path_mgr_.lock();
        auto ops_mgr  = ops_mgr_.lock();
        // First, do not create new operation of this type
        path_mgr->remove_type<OperationType>();
        // Call uninstaller
        ops_mgr->matching_op(op_type_code::get<OperationType>())->uninstall(this->shared_from_this());
        // Remove operations of this type
        ops_mgr->remove_op<OperationType>();
    }

    void install_module(priority, string_type const&);

    void uninstall_module(string_type const&);

private:
    weak_ptr<path_manager>      path_mgr_;
    weak_ptr<operation_manager> ops_mgr_;
    shared_ptr<dentry>          root = detail::shared_null_ptr;

    static shared_ptr<fs_entry> get_entry_of(shared_ptr<dentry> root, path_type const& path);
};


}   // namespace planet

#endif  // PLANET_FS_CORE_HPP
