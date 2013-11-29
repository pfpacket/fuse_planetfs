CXX        = g++
CXXFLAGS   = -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -std=c++0x -O2 -g
              -std=c++0x -O2 -g -D_FILE_OFFSET_BITS=64 -fstack-protector-all -fstack-check #-D_FORTIFY_SOURCE=1
LDFLAGS    = -shared -fPIC -rdynamic
BOOST_ROOT = /usr
INCLUDES   = -I $(BOOST_ROOT)/include -I ../../../include
LIBS       = -L $(BOOST_ROOT)/lib -lboost_system -lboost_filesystem -lboost_regex -lfuse
OBJS       = mod_dummy.o
LIBRARY    = $(PWD)/libplanet.so
TARGET     = mod_dummy.so

all: $(TARGET)
rebuild: clean all

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(LIBRARY)

.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
