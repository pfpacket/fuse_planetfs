CXX        = g++
CXXFLAGS   = -Wall -std=c++0x -O2
LDFLAGS    =
BOOST_ROOT = /usr
INCLUDES   = -I $(BOOST_ROOT)/include -I ./include
LIBS       = -L $(BOOST_ROOT)/lib -lboost_system -lboost_filesystem -lfuse
OBJS       = src/planet/common.o \
             src/planet/fs_core.o \
             src/planet/utils.o \
             src/planet/planet_handle.o \
             src/planet/basic_operation.o \
             src/planet/tcp_client_op.o \
             src/planet/tcp_server_op.o \
             src/planetfs_main.o
TARGET     = fuse_planetfs
MNTDIR     = ./net
MNTOPT     = -o direct_io -o atomic_o_trunc -o intr
MNTDBGOPT  = $(MNTOPT) -d -f -s

all: $(TARGET)
rebuild:  clean all

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

examples:
	$(MAKE) -C example/

mount: $(TARGET)
	mkdir -p $(MNTDIR)
	./$(TARGET) $(MNTOPT) $(MNTDIR)

debug_mount: $(TARGET)
	mkdir -p $(MNTDIR)
	./$(TARGET) $(MNTDBGOPT) $(MNTDIR)

umount:
	fusermount -u $(MNTDIR)

.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	$(MAKE) -C example/ clean
