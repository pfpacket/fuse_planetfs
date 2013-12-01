CXX        := g++
#CXXFLAGS  := -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wreturn-type-c-linkage -std=c++0x -O2
CXXFLAGS   := -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers \
              -std=c++0x -O2 -g $(shell pkg-config fuse --cflags) #-D_FORTIFY_SOURCE=1
LDFLAGS    := -rdynamic $(shell pkg-config fuse --libs) -fstack-protector-all -fstack-check 
BOOST_ROOT := /usr
INCLUDES   := -I $(BOOST_ROOT)/include -I ./include
LIBS       := -L $(BOOST_ROOT)/lib -lboost_system -lboost_filesystem -lfuse -lltdl
TARGET     := mount.planetfs
#OBJS       := src/planet/net/common.o \
#              src/planet/net/dns/resolver_op.o \
#              src/planet/net/tcp/common.o \
#              src/planet/net/tcp/dir_op.o \
#              src/planet/net/tcp/clone_op.o \
#              src/planet/net/tcp/ctl_op.o \
#              src/planet/net/tcp/data_op.o \
#              src/planet/net/tcp/address_op.o \
#              src/planet/net/tcp/session_dir_op.o \
#              src/planet/net/tcp/client_op.o \
#              src/planet/net/tcp/server_op.o \
#              src/planet/net/eth/raw_op.o \
#              src/planet/net/eth/dir_op.o \
#              src/planetfs_operations.o \
#              src/planetfs_main.o
OBJS       := src/planetfs_operations.o \
              src/planetfs_main.o
LIBPLANET_OBJS = \
              src/planet/common.o \
              src/planet/fs_core.o \
              src/planet/utils.o \
              src/planet/handle.o \
              src/planet/operation_layer.o \
              src/planet/basic_operation.o

DEPS       := $(OBJS:%.o=%.d)
MNTDIR     := /net
MNTOPT     := -o direct_io -o intr -o allow_other
MNTDBGOPT  := $(MNTOPT) -d -f
EXEC_ENV   := MALLOC_CHECK_=3 LD_LIBRARY_PATH=./

all: $(TARGET)

rebuild: clean all

-include $(DEPS)

$(TARGET): libplanet $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) libplanet.so $(LIBS)

libplanet: $(LIBPLANET_OBJS)
	$(CXX) $(LDFLAGS) -shared -fPIC -o $@.so $(LIBPLANET_OBJS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
#	$(CXX) -MM -MP $(CXXFLAGS) $(INCLUDES) $*.cpp > $*.d

modules:
	@$(MAKE) -C src/planet/net/dns/module/
	@$(MAKE) -C src/planet/dummy_mod/
	find src/ -type f -name "*.so" -exec cp {} . \;

mount: all
	mkdir -p $(MNTDIR)
	$(EXEC_ENV) ./$(TARGET) $(MNTOPT) $(MNTDIR)

debug_mount: all
	mkdir -p $(MNTDIR)
	$(EXEC_ENV) ./$(TARGET) $(MNTDBGOPT) $(MNTDIR)

umount:
	fusermount -u $(MNTDIR)

remount: $(TARGET) umount mount

examples:
	@$(MAKE) -C example/

test: examples
	@echo "[*] Starting test: dns_example"
	./example/dns_example www.google.com
	@echo "[*] Starting test: get_google_page.sh"
	./example/get_google_page.sh
	@echo "[*] Starting test: http_client_example"
	./example/http_client_example
	@echo "[*] Starting test: new_get_google_page.sh"
	./example/new_get_google_page.sh
	@echo "[*] Starting test: new_http_client"
	./example/new_http_client

clean:
	rm -f $(TARGET) $(OBJS)
	find . -type f -name "*.o" | xargs rm -f
	find src/ -type f -name "*.d" | xargs rm -f
	@$(MAKE) -C example/ clean
	@$(MAKE) -C src/planet/net/dns/module/ clean
	@$(MAKE) -C src/planet/dummy_mod/ clean
	@find . -maxdepth 1 -type f -name "*.so" | xargs rm -f
