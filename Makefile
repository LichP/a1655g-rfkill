ifeq ($(KERNELRELEASE),)

    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    INSTALLDIR ?= /lib/modules/$(shell uname -r)/kernel/drivers/platform/x86
    PWD := $(shell pwd)

.PHONY: module clean

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers
	
install:
	cp -v a1655g-rfkill.ko $(INSTALLDIR) && /sbin/depmod -a

else
    $(info Building with KERNELRELEASE = ${KERNELRELEASE})

    # called from kernel build system: just declare what our modules are

    obj-m :=    a1655g-rfkill.o

endif
