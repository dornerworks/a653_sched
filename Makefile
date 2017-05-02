TARGET := a653_sched

all: check_env $(TARGET)

.PHONY: check_env
check_env:
ifndef OECORE_TARGET_SYSROOT
	$(error OECORE_TARGET_SYSROOT is not set.  Was the XZD Yocto SDK sourced?)
endif

$(TARGET): $(TARGET).c
	$(CC) -o $@ $^ \
	-luuid -lxenctrl -lxenstore

clean:
	-$(RM) $(TARGET) *.o
