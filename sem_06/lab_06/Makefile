CONFIG_MODULE_SIG=n

CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)

obj-m := main.o
default: 
		make -C $(KDIR) M=$(PWD) modules 
test:
	sudo insmod main.ko
	sudo dmesg
	sudo rmmod main.ko
	sudo dmesg
clean: 
		rm -f *.o .*.cmd .*.flags *.mod.c *.order *.mod *.ko *.symvers 
		@rm -f .*.*.cmd *~ *.*~ TODO.* .*.d
		@rm -fR .tmp* 
		@rm -rf .tmp_versions 
disclean: clean 
		@rm *.ko *.symversS