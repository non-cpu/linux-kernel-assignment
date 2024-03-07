obj-m := ftracehooking.o iotracehooking.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc test.c
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
