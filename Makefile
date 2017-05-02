TARGET := a653_sched

all: check_env $(TARGET)

CC := $(shell find /opt/Xilinx -iname aarch64-linux-gnu-gcc | sort -Vr | head -1)

.PHONY: check_env
check_env:
ifndef OECORE_TARGET_SYSROOT
	$(error OECORE_TARGET_SYSROOT is not set.  Was the XZD Yocto SDK sourced?)
endif
	@if [ -z "$(CC)" ]; then \
		echo "No Xilinx SDK Toolchain Found"; \
		false; \
	else \
		echo "Using Xilinx SDK Toolchain at $(CC)"; \
	fi

$(TARGET): $(TARGET).c
	$(CC) -o $@ $^ \
	--sysroot=$(OECORE_TARGET_SYSROOT) \
	-luuid -lxenctrl -lxenstore

clean:
	-$(RM) $(TARGET) *.o
