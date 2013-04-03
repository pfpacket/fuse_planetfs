CXX        = g++
CXXFLAGS   = -Wall -std=c++0x -O0 -g
LDFLAGS    =
BOOST_ROOT = /usr
INCLUDES   = -I $(BOOST_ROOT)/include -I ./include
LIBS       = -L $(BOOST_ROOT)/lib -lboost_system -lboost_filesystem -lfuse
OBJS       = src/planetfs_main.o
TARGET     = fuse_planetfs

all: $(TARGET)
rebuild:  clean all

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

test.out: test/test.o
	$(CXX) $(LDFLAGS) -o $@ test/test.o $(LIBS)

mount: $(TARGET)
	mkdir -p net/
	./fuse_planetfs -o direct_io net/

umount:
	fusermount -u net/

.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) test.out test/test.o
