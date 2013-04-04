#ifndef PLANET_COMMON_HPP
#define PLANET_COMMON_HPP

#ifndef FUSE_USE_VERSION
    #define FUSE_USE_VERSION 26
#endif

#ifndef _FILE_OFFSET_BITS
    #define _FILE_OFFSET_BITS 64
#endif

#include <fuse/fuse.h>
#include <syslog.h>

// namespace for planetfs
namespace planet {
}   // namespace planet

#include <planet/exception.hpp>

#endif  // PLANET_COMMON_HPP
