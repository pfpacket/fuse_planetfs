#
# Makefile
#
ROOT       = ../../..
INCLUDES  += -I $(ROOT)/include
LIBS      += -L ./ -lplanet
OBJS       = mod_dummy.o
TARGET     = mod_dummy.so

all: $(TARGET)
rebuild: clean all

libplanet:
	@ln -sf $(ROOT)/libplanet.so ./libplanet.so

$(TARGET): libplanet $(OBJS)
	$(CXX) $(LDFLAGS) -shared -fPIC -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) -MMD -MP -MT $@ $(CXXFLAGS) $(INCLUDES) -c $< -o $@
-include $(DEPS)

clean:
	rm -f $(TARGET) $(OBJS) ./libplanet.so
