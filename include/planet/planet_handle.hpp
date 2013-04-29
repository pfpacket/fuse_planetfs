#ifndef PLANET_SOCKET_HPP
#define PLANET_SOCKET_HPP

#include <planet/common.hpp>
#include <map>
#include <deque>
#include <mutex>
#include <memory>
#include <utility>
#include <cerrno>
#include <planet/basic_operation.hpp>

namespace planet {


// planet handle type
typedef int handle_t;

class planet_handle_manager {
private:
    std::mutex mtx_;
    typedef std::lock_guard<decltype(mtx_)> lock_guard;
    handle_t current_;
    typedef std::tuple<
        shared_ptr<file_entry>,
        shared_ptr<planet_operation>
    > entry_type;
    std::map<handle_t, entry_type> ops_;

public:
    planet_handle_manager(int init = 0);

    entry_type const& get_operation_entry(handle_t ph)
    {
        return ops_.at(ph);
    }

    template<typename ...Types>
    handle_t register_op(shared_ptr<file_entry> fp, opcode op, Types&& ...args)
    {
        shared_ptr<planet_operation> op_ptr{};
        if (op == opcode::default_op)
            op_ptr = std::make_shared<default_file_op>(std::forward<Types>(args)...);
        lock_guard lock(mtx_);
        ops_.insert(
            std::make_pair(
                current_ + 1,
                std::make_tuple(fp, op_ptr)
            )
        );
        return ++current_;
    }

    void unregister_op(handle_t ph);
};

class planet_path_manager {
public:
    typedef std::function<bool(path_type const&)> comp_func_t;
    typedef std::tuple<opcode, comp_func_t> value_type;

    void register_op(opcode op, comp_func_t comp)
    {
        registry_.emplace_back(op, comp);
    }

    void unregister_op(opcode op)
    {
        auto it = std::remove_if(registry_.begin(), registry_.end(),
            [op](value_type const& v) { return std::get<0>(v) == op; });
        registry_.erase(it, registry_.end());
    }

    opcode find_path_op(path_type const& path)
    {
        auto it = std::find_if(registry_.begin(), registry_.end(),
            [path](value_type const& v) { return std::get<1>(v)(path); });
        if (it == registry_.end())
            throw planet::exception_errno(EBADR);
        return std::get<0>(*it);
    }
private:
    std::deque<value_type> registry_;
};

extern planet_handle_manager handle_mgr;
extern planet_path_manager path_mgr;

handle_t get_handle_from(struct fuse_file_info const& fi);

void set_handle_to(struct fuse_file_info& fi, handle_t ph);

}   // namespace planet

#endif  // PLANET_SOCKET_HPP
