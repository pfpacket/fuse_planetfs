#ifndef PLANETFS_OPERATIONS_HPP
#define PLANETFS_OPERATIONS_HPP

#include <planet/filesystem.hpp>

// Core filesystem object
extern std::unique_ptr<planet::filesystem> fs;

extern void planet_attach(Ixp9Req*);
extern void planet_clunk(Ixp9Req*);
extern void planet_create(Ixp9Req*);
extern void planet_flush(Ixp9Req*);
extern void planet_open(Ixp9Req*);
extern void planet_read(Ixp9Req*);
extern void planet_remove(Ixp9Req*);
extern void planet_stat(Ixp9Req*);
extern void planet_walk(Ixp9Req*);
extern void planet_write(Ixp9Req*);
extern void planet_wstat(Ixp9Req*);
extern void planet_freefid(IxpFid*);


#endif  // PLANETFS_OPERATIONS_HPP
