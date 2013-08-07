CXX        = g++
#CXXFLAGS   = -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -std=c++0x -O2 -march=native
CXXFLAGS   = -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -std=c++0x -O1 -g -D_FORTIFY_SOURCE=1
LDFLAGS    = -rdynamic
BOOST_ROOT = /usr
INCLUDES   = -I $(BOOST_ROOT)/include -I ./include
LIBS       = -L $(BOOST_ROOT)/lib -lboost_system -lboost_filesystem -lboost_regex -lfuse -ldl
OBJS       = src/planet/common.o \
             src/planet/fs_core.o \
             src/planet/utils.o \
             src/planet/handle.o \
             src/planet/operation_layer.o \
             src/planet/basic_operation.o \
             src/planet/dyn_module_op.o \
             src/planet/net/common.o \
             src/planet/net/dns/resolver_op.o \
             src/planet/net/tcp/common.o \
             src/planet/net/tcp/dir_op.o \
             src/planet/net/tcp/clone_op.o \
             src/planet/net/tcp/ctl_op.o \
             src/planet/net/tcp/data_op.o \
             src/planet/net/tcp/address_op.o \
             src/planet/net/tcp/session_dir_op.o \
             src/planet/net/tcp/client_op.o \
             src/planet/net/tcp/server_op.o \
             src/planet/net/eth/raw_op.o \
             src/planet/net/eth/dir_op.o \
             src/planetfs_operations.o \
             src/planetfs_main.o
TARGET     = mount.planetfs
MNTDIR     = /net
MNTOPT     = -o direct_io \
             -o intr -o allow_other
MNTDBGOPT  = $(MNTOPT) -d -f
EXEC_ENV   = LD_LIBRARY_PATH=./ MALLOC_CHECK_=3

all: $(TARGET) modules
rebuild: clean all

modules:
	$(MAKE) -C src/planet/net/dns/module/
	@find src/ -type f -name "*.so" -exec cp {} . \;

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

examples:
	$(MAKE) -C example/

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

mount: $(TARGET) modules
	mkdir -p $(MNTDIR)
	$(EXEC_ENV) ./$(TARGET) $(MNTOPT) $(MNTDIR)

debug_mount: $(TARGET)
	mkdir -p $(MNTDIR)
	$(EXEC_ENV) ./$(TARGET) $(MNTDBGOPT) $(MNTDIR)

umount:
	fusermount -u $(MNTDIR)

remount: $(TARGET) umount mount

.cpp.o: 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	$(MAKE) -C example/ clean
	$(MAKE) -C src/planet/net/dns/module/ clean
	@find . -maxdepth 1 -type f -name "*.so" | xargs rm -f
