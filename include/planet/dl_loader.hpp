#ifndef PLANET_DL_LOADER
#define PLANET_DL_LOADER

#include <planet/common.hpp>
#include <ltdl.h>

namespace planet {


    class dl_loader {
    private:
        lt_dlhandle handle_ = nullptr;

        static void ltdl_init()
        {
            if (::lt_dlinit())
                throw_ltdl_error<>();
        }

        static void ltdl_exit() noexcept
        {
            ::lt_dlexit();
        }

        static lt_dlhandle ltdl_open(string_type const& dl_name)
        {
            auto handle = ::lt_dlopen(dl_name.c_str());
            if (!handle)
                throw_ltdl_error<>();
            return handle;
        }

        static void ltdl_close(lt_dlhandle& handle) noexcept
        {
            if (handle)
                ::lt_dlclose(handle);
            handle = nullptr;
        }

        static void ltdl_add_searchdir(std::vector<string_type> const& path_list)
        {
            for (auto&& path : path_list)
                ::lt_dladdsearchdir(path.c_str());
        }

        void make_sure_module_loaded() const
        {
            if (!handle_)
                throw std::runtime_error("dl_loader: module not loaded");
        }

        template<typename Exception = exception_errno>
        static void throw_ltdl_error()
        {
            throw Exception{ELIBACC, str(format("%1%: ") % ::lt_dlerror())};
        }

    public:
        dl_loader()
        {
            ltdl_init();
        }

        dl_loader(string_type const& dl_name)
        {
            ltdl_init();
            try {
                handle_ = ltdl_open(dl_name);
            } catch (...) {
                ltdl_exit();
                throw;
            }
        }

        dl_loader(string_type const& dl_name, std::vector<string_type> const& path_list)
        {
            ltdl_init();
            try {
                ltdl_add_searchdir(path_list);
                handle_ = ltdl_open(dl_name);
            } catch (...) {
                ltdl_exit();
                throw;
            }
        }

        ~dl_loader()
        {
            ltdl_close(handle_);
            ltdl_exit();
        }

        void add_searchdir(std::vector<string_type> const& path_list)
        {
            ltdl_add_searchdir(path_list);
        }

        void load_module(string_type const& dl_name)
        {
            ltdl_close(handle_);
            handle_ = ltdl_open(dl_name);
        }

        void *load_symbol(string_type const& symname) const
        {
            this->make_sure_module_loaded();
            void *sym_addr = ::lt_dlsym(handle_, symname.c_str());
            if (!sym_addr)
                throw_ltdl_error<>();
            return sym_addr;
        }

        template<typename FunctionType>
        std::function<FunctionType> load_function(string_type const& funcname) const
        {
            auto *sym_addr
                = reinterpret_cast<typename std::add_pointer<FunctionType>::type>
                    (this->load_symbol(funcname));
            return sym_addr;
        }

        ::lt_dlinfo const& info() const
        {
            this->make_sure_module_loaded();
            return *::lt_dlgetinfo(handle_);
        }
    };


}   // namespace planet

#endif  // PLANET_DL_LOADER
