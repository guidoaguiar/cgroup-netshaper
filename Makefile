obj-m += cgroup_netshaper.o
cgroup_netshaper-objs := src/netshaper_main.o src/netshaper_cgroup.o src/netshaper_netfilter.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

install:
	make -C $(KERNELDIR) M=$(PWD) modules_install
	depmod -a

uninstall:
	rm -f /lib/modules/$(shell uname -r)/extra/cgroup_netshaper.ko
	depmod -a

.PHONY: all clean install uninstall