#
# Makefile
#
ROOT        = ../../..
CXXFLAGS   += -I $(ROOT)/include
OBJS        = mod_dummy.o
TARGET      = mod_dummy.so

all: $(TARGET)
rebuild: clean all

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -shared -fPIC -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) -MMD -MP -MT $@ $(CXXFLAGS) $(INCLUDES) -c $< -o $@
-include $(DEPS)

clean:
	rm -f $(TARGET) $(OBJS)
