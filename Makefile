obj-m += cgroup_netshaper.o
cgroup_netshaper-objs := src/netshaper_main.o src/netshaper_cgroup.o src/netshaper_netfilter.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Additional CFLAGS for kernel version compatibility
EXTRA_CFLAGS += -Wall -Wno-unused-function
EXTRA_CFLAGS += -DKBUILD_MODNAME=\"cgroup_netshaper\"

# Check for kernel version to handle API differences
KERNEL_VERSION := $(shell uname -r | cut -d. -f1-2)
KVER_CODE := $(shell printf "%d%02d00" $$(echo $(KERNEL_VERSION) | cut -d. -f1) $$(echo $(KERNEL_VERSION) | cut -d. -f2))

ifneq (,$(shell grep -l "nf_register_net_hooks" $(KERNELDIR)/include/linux/netfilter.h 2>/dev/null))
    EXTRA_CFLAGS += -DHAS_NET_HOOKS
endif

ifneq (,$(shell grep -l "sock_gen_put" $(KERNELDIR)/include/net/sock.h 2>/dev/null))
    EXTRA_CFLAGS += -DHAS_SOCK_GEN_PUT
endif

all:
	make -C $(KERNELDIR) M=$(PWD) modules

sparse:
	make -C $(KERNELDIR) M=$(PWD) C=2 CF="-D__CHECK_ENDIAN__" modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

install:
	make -C $(KERNELDIR) M=$(PWD) modules_install
	depmod -a

uninstall:
	rm -f /lib/modules/$(shell uname -r)/extra/cgroup_netshaper.ko
	depmod -a

.PHONY: all clean install uninstall sparse