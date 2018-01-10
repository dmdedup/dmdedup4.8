obj-m += dm-dedup.o
KDIR ?= /lib/modules/$(shell uname -r)/build

dm-dedup-objs := dm-dedup-cbt.o dm-dedup-hash.o dm-dedup-ram.o dm-dedup-check.o dm-dedup-gc.o dm-dedup-rw.o dm-dedup-target.o

EXTRA_CFLAGS := -Idrivers/md

all:
	make -C $(KDIR)  M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
