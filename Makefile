CXX        = g++
CXXFLAGS   = -Wall -std=c++0x -O2
LDFLAGS    =
BOOST_ROOT = /usr
INCLUDES   = -I $(BOOST_ROOT)/include -I ./include
LIBS       = -L $(BOOST_ROOT)/lib -lboost_system -lboost_filesystem -lfuse
OBJS       = src/planetfs_main.o \
             src/planet/common.o \
             src/planet/fs_core.o \
             src/planet/utils.o \
             src/planet/planet_handle.o \
             src/planet/basic_operation.o
TARGET     = fuse_planetfs

all: $(TARGET)
rebuild:  clean all

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

examples:
	$(MAKE) -C example/

mount: $(TARGET)
	mkdir -p net/
	./fuse_planetfs -o direct_io net/

umount:
	fusermount -u net/

.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	$(MAKE) -C example/ clean
