
usb-teensy-objs	:= main.o file_ops.o

obj-m	:= usb-teensy.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

all: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) 

install: all
	mkdir -p /lib/modules/`(uname -r)`/teensy
	cp usb-teensy.ko /lib/modules/`(uname -r)`/teensy
	/sbin/depmod

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
	rm -f Module.symvers modules.order
	rm -rf /lib/modules/`(uname -r)`/teensy

