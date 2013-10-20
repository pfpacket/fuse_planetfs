CXX        = g++
CXXFLAGS   = -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -std=c++0x -O2 -march=native
LDFLAGS    = -shared -fPIC -rdynamic
BOOST_ROOT = /usr
INCLUDES   = -I $(BOOST_ROOT)/include -I ../../../include
LIBS       = -L $(BOOST_ROOT)/lib -lboost_system -lboost_filesystem -lboost_regex -lfuse
OBJS       = ../../../src/planet/common.o \
             ../../../src/planet/fs_core.o \
             ../../../src/planet/utils.o \
             ../../../src/planet/handle.o \
             ../../../src/planet/operation_layer.o \
             ../../../src/planet/basic_operation.o \
             ../../../src/planet/net/common.o \
             ../../../src/planet/net/dns/resolver_op.o \
             mod_dummy.o
TARGET     = mod_dummy.so

all: $(TARGET)
rebuild: clean all

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
