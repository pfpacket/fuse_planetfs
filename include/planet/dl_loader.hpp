#ifndef PLANET_DL_LOADER
#define PLANET_DL_LOADER

#include <planet/common.hpp>
#include <ltdl.h>

// See also:
// http://stackoverflow.com/questions/14543801/iso-c-forbids-casting-between-pointer-to-function-and-pointer-to-object
// http://agram66.blogspot.jp/2011/10/dlsym-posix-c-gimme-break.html
// http://stackoverflow.com/questions/3941793/what-is-guaranteed-about-the-size-of-a-function-pointer/3941867#3941867
static_assert(sizeof(void *) == sizeof(void (*)()),
    "ISO C++ forbids casting between pointer-to-function and pointer-to-object"
    " and the following is not guaranteed on your system, "
    "which means using dlsym() to load functions is not safe anymore: "
    "sizeof(void *) == sizeof(void (*)()");

namespace planet {


    class dl_loader {
    private:
        lt_dlhandle handle_ = nullptr;

        static void ltdl_init()
        {
            if (::lt_dlinit())
                throw_ltdl_error(EIO);
        }

        static void ltdl_exit() noexcept
        {
            ::lt_dlexit();
        }

        static lt_dlhandle ltdl_open(string_type const& dl_name)
        {
            auto handle = ::lt_dlopenext(dl_name.c_str());
            if (!handle)
                throw_ltdl_error(ELIBACC, dl_name);
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

        static void throw_ltdl_error(int errc)
        {
            throw_system_error(errc, ::lt_dlerror());
        }

        static void throw_ltdl_error(int errc, string_type const& prefix)
        {
            throw_system_error(errc, prefix + ": " + ::lt_dlerror());
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

        dl_loader& operator=(dl_loader const& dl)
        {
            if (dl.handle_) {
                if (this->handle_)
                    ltdl_close(this->handle_);
                this->handle_ = ltdl_open(dl.info().name);
            }
            return *this;
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
                throw_ltdl_error(ELIBBAD);
            return sym_addr;
        }

        template<typename FuncPtrType>
        FuncPtrType load_func_ptr(
            string_type const& funcname,
            typename std::enable_if<std::is_pointer<FuncPtrType>::value>::type* = 0) const
        {
            return reinterpret_cast<FuncPtrType>(this->load_symbol(funcname));
        }

        template<typename FunctionType,
            typename FuncPtrType = typename std::add_pointer<FunctionType>::type>
        FuncPtrType load_func_ptr(
            string_type const& funcname,
            typename std::enable_if<!std::is_pointer<FunctionType>::value>::type* = 0) const
        {
            return this->load_func_ptr<FuncPtrType>(funcname);
        }

        template<typename FunctionType>
        std::function<FunctionType> load_function(string_type const& funcname) const
        {
            return this->load_func_ptr<FunctionType>(funcname);
        }

        ::lt_dlinfo const& info() const
        {
            this->make_sure_module_loaded();
            return *::lt_dlgetinfo(handle_);
        }
    };


}   // namespace planet

#endif  // PLANET_DL_LOADER
