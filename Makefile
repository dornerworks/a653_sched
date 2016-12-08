TARGET := a653_sched

all: check_env $(TARGET)

.PHONY: check_env
check_env:
ifndef RELEASE_DIR
	$(error RELEASE_DIR is not set)
endif

BUILD_ROOT_DIR := $(RELEASE_DIR)/buildroot

CC := $(BUILD_ROOT_DIR)/output/host/usr/bin/aarch64-buildroot-linux-gnu-gcc
XENPATH := $(RELEASE_DIR)/XenZynqDist/components/apps/xen/xen-src/
SYSROOT := $(BUILD_ROOT_DIR)/output/host/usr/aarch64-buildroot-linux-gnu/sysroot/
LDPATH := $(XENPATH)/dist/install/usr/local/lib/

$(TARGET): $(TARGET).c
	$(CC) --sysroot=$(SYSROOT) -I$(XENPATH)tools/include/ -I$(XENPATH)tools/libxc/include/ -I$(XENPATH)tools/xenstore/include/ -I$(XENPATH)tools/libs/toollog/include/ -o $@ $^ -L$(LDPATH) -luuid -lxenctrl -lxentoollog -lxenevtchn -lxengnttab -lxencall -lxenforeignmemory -lxenstore

clean:
	-$(RM) $(TARGET) *.o
