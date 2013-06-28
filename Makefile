CXX        = g++
CXXFLAGS   = -Wall -std=c++0x -O2
LDFLAGS    =
BOOST_ROOT = /usr
INCLUDES   = -I $(BOOST_ROOT)/include -I ./include
LIBS       = -L $(BOOST_ROOT)/lib -lboost_system -lboost_filesystem -lfuse
OBJS       = src/planet/common.o \
             src/planet/fs_core.o \
             src/planet/utils.o \
             src/planet/handle.o \
             src/planet/operation_layer.o \
             src/planet/basic_operation.o \
             src/planet/dns/dns_op.o \
             src/planet/tcp/client_op.o \
             src/planet/tcp/server_op.o \
             src/planet/eth/raw_op.o \
             src/planetfs_operations.o \
             src/planetfs_main.o
TARGET     = mount.planetfs
MNTDIR     = /net
MNTOPT     = -o direct_io -o atomic_o_trunc \
             -o intr -o allow_other
MNTDBGOPT  = $(MNTOPT) -d -f -s

all: $(TARGET)
rebuild: clean all

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

remount: $(TARGET) umount mount

.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	$(MAKE) -C example/ clean
