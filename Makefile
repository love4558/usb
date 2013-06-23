obj-m		:= systec_can.o
KDIR 		:= /lib/modules/$(shell uname -r)/build
PWD		:= $(shell pwd)

all:
		$(MAKE) -C $(KDIR) M=$(PWD)

clean:
		$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean
