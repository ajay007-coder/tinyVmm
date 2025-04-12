obj-m += tinyVmm.o


INC_DIR := $(PWD)/includes

tinyVmm-objs := src/tinyVmm.o src/cpu/vmx.o

KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) EXTRA_CFLAGS=-I$(INC_DIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean