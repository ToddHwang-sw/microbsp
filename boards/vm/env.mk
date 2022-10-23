
## H/W core 
export _CORE_=x86_64

## architecture name 
export _ARCH_=$(_CORE_)-vm

## platfom string ...
export PLATFORM=$(_CORE_)-any-linux-gnu

## kernel version 
export KERNELVER=linux-5.17.7

## Linux Kernel compile
export KERNELMAKE=make ARCH=$(_CORE_) CROSS_COMPILE=$(CROSS_COMP_PREFIX) 

##
## folders to be included
##
export extra_SUBDIR=\
		lpps

##
## folders to be included
##
export extra_SUBDIR=

##
## external disk volume name 
##
export EXTDISK=apps

##
## external disk image file name 
##
export EXTDISKNM=image.ext4

##
## size of external disk : 1M x 6K = 6G
##
export EXTDISKBLKS=6K

##
## Configuration disk image file name
##
export CFGDISKNM=config.ext4

##
## size of external disk : 1M x 10 = 10M
##
export CFGDISKBLKS=10

##
## UI disk volume name 
##
export UIDISK=

##
## UI disk image file name 
##
export UIDISKNM=

##
## size of UI disk : 1M x 6K = 6G
##
export UIDISKBLKS=

##
## Read-only Squashfs file systems 
##
export SQUASHROOTFS=y

##
## This system is booted by QEMU at ttyS0 line serial basis. 
## Terminal should be reopened tty0 graphic interface ... 
##
export TTY0_BASH=/bin/bash -l
## export TTY0_BASH=/sbin/getty -l /bin/bash -n 0 tty0

##
##  ATTENTION !! 
##
## sysinit:/etc/init.d/rcS    --> called at booting time from booting disk 
## shutdown:/etc/rc.shutdown  --> called at off time from overlayed space (boards/vm/rootfs/etc/rc.shutdown)
##
##
export BUILDUP_ROOTFS=\
	[ -d $(XBASEDIR)/proc ]       || mkdir -p $(XBASEDIR)/proc  && \
	[ -d $(XBASEDIR)/var  ]       || mkdir -p $(XBASEDIR)/var   && \
	[ -d $(XBASEDIR)/dev  ]       || mkdir -p $(XBASEDIR)/dev   && \
	[ -d $(XBASEDIR)/sys  ]       || mkdir -p $(XBASEDIR)/sys   && \
	[ -d $(XBASEDIR)/mnt  ]       || mkdir -p $(XBASEDIR)/mnt   && \
	[ -d $(XBASEDIR)/low2 ]       || mkdir -p $(XBASEDIR)/low2  && \
	[ -d $(XBASEDIR)/ovr  ]       || mkdir -p $(XBASEDIR)/ovr   && \
	[ -d $(XBASEDIR)/ovr/dev  ]   || mkdir -p $(XBASEDIR)/ovr/dev      && \
	[ -d $(XBASEDIR)/ovr/proc ]   || mkdir -p $(XBASEDIR)/ovr/proc     && \
	[ -s $(XBASEDIR)/tmp  ]       || ln -s /var/tmp  $(XBASEDIR)/tmp   && \
	[ -s $(XBASEDIR)/root ]       || ln -s /var/root $(XBASEDIR)/root  && \
	[ ! -d $(XBASEDIR)/lib    ]   || \rm -rf $(XBASEDIR)/lib       && \
	[ ! -d $(XBASEDIR)/lib64  ]   || \rm -rf $(XBASEDIR)/lib64     && \
	[ ! -d $(XBASEDIR)/etc    ]   || \rm -rf $(XBASEDIR)/etc       && \
	[ -d $(XBASEDIR)/lib    ]     || mkdir -p $(XBASEDIR)/lib        && \
	[ -d $(XBASEDIR)/etc    ]     || mkdir -p $(XBASEDIR)/etc        && \
	[ -d $(XBASEDIR)/root   ]     || mkdir -p $(XBASEDIR)/root       && \
	[ -d $(XBASEDIR)/etc/init.d ] || mkdir -p $(XBASEDIR)/etc/init.d                   && \
	[ -d $(XBASEDIR)/lib64      ] || mkdir -p $(XBASEDIR)/lib64                        && \
	echo "ttyS0::respawn:$(TTY0_BASH)"              >  $(XBASEDIR)/etc/inittab         && \
	echo "::sysinit:/etc/init.d/rcS"                >> $(XBASEDIR)/etc/inittab         && \
	echo "::shutdown:/etc/rc.shutdown"              >> $(XBASEDIR)/etc/inittab         && \
	echo "\#!/bin/bash "                            >  $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount -t proc proc /proc "                >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount -t tmpfs ram /var  -o size=4m "     >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount -t sysfs sys /sys "                 >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mkdir -p /var/tmp "                       >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mkdir -p /var/root "                      >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mkdir -p /var/udir"                       >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mkdir -p /var/wdir"                       >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "export PATH=/bin:/sbin:/usr/bin:/usr/sbin">> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "export LD_LIBRARY_PATH=/lib:/lib64:/usr/lib:/usr/lib64 "  \
                                                	>> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount /dev/sda /mnt"                      >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "[ -d /mnt/usr  ] || ( echo \"External disk is not mounted !!\" ; exit 1 ) " \
							                        >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "[ -d /mnt/work ] || ( echo \"Work disk is not mounted !!\" ; exit 1 ) " \
							                        >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "echo \"Mounting...\" "                    >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount -t overlay -o lowerdir=/disk,upperdir=/mnt/usr,workdir=/mnt/work overlay /ovr" \
							                        >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "Done" > /dev/null 

