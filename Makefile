CXX        = g++
CXXFLAGS   = -Wall -std=c++0x -O0 -g
LDFLAGS    =
BOOST_ROOT = /usr
INCLUDES   = -I $(BOOST_ROOT)/include -I ./include
LIBS       = -L $(BOOST_ROOT)/lib -lboost_system -lboost_filesystem -lfuse
OBJS       = pnetfs.o
TARGET     = fuse_pnetfs

all: $(TARGET)
rebuild:  clean all

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

test: test.o
	$(CXX) $(LDFLAGS) -o $@ test.o $(LIBS)

.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) test test.o
