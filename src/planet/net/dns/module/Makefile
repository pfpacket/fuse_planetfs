#
# Makefile
#
ROOT       = ../../../../..
INCLUDES  += -I $(ROOT)/include
LIBS      += -L ./ -lplanet
OBJS       = mod_net_dns.o \
             $(ROOT)/src/planet/net/common.o \
             $(ROOT)/src/planet/net/dns/resolver_op.o
TARGET     = mod_net_dns.so

all: prepare $(TARGET)
rebuild: clean all

prepare:
	@ln -sf $(ROOT)/libplanet.so ./libplanet.so

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -shared -fPIC -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) -MMD -MP -MT $@ $(CXXFLAGS) $(INCLUDES) -c $< -o $@
-include $(DEPS)

clean:
	rm -f $(TARGET) $(OBJS) ./libplanet.so
