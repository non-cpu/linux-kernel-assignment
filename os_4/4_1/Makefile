obj-m := file_varea.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc -o test test.c -static

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f test

load:
	sudo insmod file_varea.ko; ./test; dmesg; sudo rmmod file_varea

unload:
	sudo rmmod file_varea
