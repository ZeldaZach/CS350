CONFIG_MODULE_SIG=n

# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile there then includes this
# Makefile once again.

# This conditional selects whether we are being included from the
# kernel Makefile or not.
ifeq ($(KERNELRELEASE),)

	# Assume the source tree is where the running kernel was built
	# You should set KERNELDIR in the environment if it's elsewhere
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build

	# The current directory is passed to sub-makes as argument
	PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	gcc producer_numbers.c -o producer_numbers
	gcc consumer_numbers.c -o consumer_numbers

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *_numbers

.PHONY: modules modules_install clean

else
	# called from kernel build system: just declare what our modules are
	obj-m := numpipe.o

	# Compile against C11
    ccflags-y := -std=gnu11
endif
