obj-m += bitscore.o
obj-m += bitsfeed.o

# bitscore.ko is built from two source files (kbuild multi-file syntax)
bitscore-objs := core.o stats.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
	depmod -a

.PHONY: all clean install
