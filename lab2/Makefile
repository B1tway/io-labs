NAME=vram_disk_dev
DNAME=vramdisk
MAKE=make
KDIR?=/lib/modules/$(shell uname -r)/build
# specify your number of logic cores
nproc?=8

obj-m += $(NAME).o
$(NAME)-objs := src/_$(NAME).o # add here additional object files for compiling

all:
	make -j $(nproc) -C $(KDIR) M=$(shell pwd) modules

clean:
	make -j $(nproc) -C $(KDIR) M=$(shell pwd) clean

load:
	sudo insmod $(NAME).ko

unload:
	sudo rmmod $(NAME).ko

vfat:
	sudo mkfs.vfat $(DPART)

ls:
	@sudo lsblk | head -n 1;
	@sudo lsblk | grep -E $(DNAME)

.PHONY: clean load unload ls
