#
# Makefile
#
ROOT       = ../../../../..
INCLUDES  += -I $(ROOT)/include
LIBS      += -L $(ROOT) -lplanet
OBJS       = mod_net_eth.o \
             $(ROOT)/src/planet/net/common.o \
             $(ROOT)/src/planet/net/eth/dir_op.o \
             $(ROOT)/src/planet/net/eth/raw_op.o
TARGET     = mod_net_eth.so

all: $(TARGET)
rebuild: clean all

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -shared -fPIC -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) -MMD -MP -MT $@ $(CXXFLAGS) $(INCLUDES) -c $< -o $@
-include $(DEPS)

clean:
	rm -f $(TARGET) $(OBJS)
