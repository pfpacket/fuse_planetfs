#ifndef PLANET_UTILS_HPP
#define PLANET_UTILS_HPP

#include <planet/common.hpp>

namespace planet {


    class fs_entry;
    class file_entry;
    class dentry;
    class core_file_system;

    // Cast entry to file_entry
    // Exception: If `entry` is not a type of file_entry
    //          : Also shared_ptr may throw exceptions
    shared_ptr<file_entry> file_cast(shared_ptr<fs_entry> entry);

    // Cast entry to dentry
    // Exception: If `entry` is not a type of dentry
    //          : Also shared_ptr may throw exceptions
    shared_ptr<dentry> directory_cast(shared_ptr<fs_entry> entry);

    template<typename Type, typename VecType>
    void store_data_to_vector(std::vector<VecType>& buffer, Type const& data)
    {
        buffer.reserve(sizeof (Type));
        *reinterpret_cast<Type *>(buffer.data()) = data;
    }

    template<typename Type, typename VecType>
    Type& get_data_from_vector(std::vector<VecType>& buffer)
    {
        if (buffer.capacity() < sizeof (Type))
            throw std::out_of_range(
                "get_data_from_vector(): buffer.capacity() < sizeof (Type)"
            );
        return *reinterpret_cast<Type *>(buffer.data());
    }

    template<typename T, typename D>
    std::unique_ptr<T, D> make_unique_ptr(T *p, D d) noexcept
    {
        return std::unique_ptr<T, D>(p, std::forward<D>(d));
    }

    // Equivalent to `return file_cast(fs_root.get_entry_of())`
    //  with exception handling
    shared_ptr<file_entry> search_file_entry(core_file_system const&, path_type const&);

    // Equivalent to `return dir_cast(fs_root.get_entry_of())`
    //  with exception handling
    shared_ptr<dentry> search_dir_entry(core_file_system const&, path_type const&);

    class raii_wrapper {
    private:
        typedef std::function<void (void)> functor_t;
        functor_t finalizer_;

    public:
        raii_wrapper() = default;

        template<typename Functor, typename ...Types>
        raii_wrapper(Functor f, Types&& ...args)
            :   finalizer_{
                    std::bind(std::forward<Functor>(f), std::forward<Types>(args)...)
                }
        {
        }

        raii_wrapper(raii_wrapper const&) = delete;
        raii_wrapper& operator=(raii_wrapper const&) = delete;

        raii_wrapper(raii_wrapper&& r);
        raii_wrapper& operator=(raii_wrapper &&);

        ~raii_wrapper();

        template<typename Functor, typename ...Types>
        void set_finalizer(Functor f, Types&& ...args)
        {
            finalizer_ = std::bind(std::forward<Functor>(f), std::forward<Types>(args)...);
        }

        functor_t const& get_finalizer() const;

        void finalize() const;
    };

}   // namespace planet

#endif  // PLANET_UTILS_HPP
