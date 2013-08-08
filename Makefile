obj-m := sbd.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all:
	make -C $(KDIR) M=`pwd` SUBDIRS=$(PWD) modules

