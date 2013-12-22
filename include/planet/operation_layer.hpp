#ifndef PLANET_OPERATION_LAYER
#define PLANET_OPERATION_LAYER

#include <planet/common.hpp>
#include <planet/handle.hpp>

namespace planet {


    // read(2) system call entry
    extern int read(handle_t, char *, size_t, off_t);

    // write(2) system call entry
    extern int write(handle_t, char const *, size_t, off_t);

    // close(2) system call entry
    extern int close(handle_t);

    // Poll handle
    extern int poll(handle_t, pollmask_t& pollmask);


}   // namespace planet

#endif  // PLANET_OPERATION_LAYER
