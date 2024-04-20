
## H/W core 
export _CORE_=arm

## architecture name 
export _ARCH_=$(_CORE_)-tinker

## platfom string ...
##
## NOTICE !! The should be ended with "*eabi" only for the case of arm 32 bit architecture
##
export PLATFORM=$(_CORE_)-mbsp-linux-gnueabi

## kernel version 
export KERNELVER=linux-5.15.113

## Linux Kernel compile
export KERNELMAKE=make ARCH=arm V=1 CROSS_COMPILE=$(CROSS_COMP_PREFIX) CFLAGS_KERNEL="$(CROSS_COMP_FLAGS)" CFLAGS_MODULE="$(CROSS_COMP_FLAGS)"

##
## folders to be included
##
export extra_SUBDIR=\
		wpa \
		wtools \
		hostapd \
		dhcpv6 \
		ipv6calc \
		libmnl \
		libnftnl \
		nftables \
		iptables \
		iproute2 \
		dhcp \
		radvd \
		dnsmasq \
		i2ctool \
		gpio

##
## Kernel image tar.gz
##
export BDTARGZ=boot.tgz

##
## external disk volume name 
##
export EXTDISK=apps

##
## external disk image file name 
##
export EXTDISKNM=image.ext4

##
## size of external disk : 1M x 8K = 8G
##
export EXTDISKBLKS=8K

##
## Configuration disk image file name
##
export CFGVOLNM=config

##
## Configuration disk image file name
##
export CFGDISKNM=$(CFGVOLNM).ext4

##
## size of external disk : 1M x 10 = 10M
##
export CFGDISKBLKS=10

##
## UI disk volume name 
##
export UIDISK=ui

##
## UI disk image file name 
##
export UIDISKNM=ui.ext4

##
## size of UI disk : 1M x 4K = 4G
##
export UIDISKBLKS=4K

##
## Read-only Squashfs file systems 
##
export SQUASHROOTFS=y

##
##  ATTENTION !! 
##
##     /dev/mmcblk0p3 - /mnt  : Main lower layer (system part - should be used)
##     /dev/mmcblk0p5 - /ui   : Partition collecting all binaries from /uix 
##     /dev/mmcblk0p6 - /root : Will be mounted in /etc/rc.init
##     /dev/mmcblk0p7 - /low2 : Third lower layer    (Updatable partition) - undefined 
##
export BUILDUP_ROOTFS=\
	[ -d $(XBASEDIR)/proc ] || mkdir -p $(XBASEDIR)/proc  && \
	[ -d $(XBASEDIR)/var  ] || mkdir -p $(XBASEDIR)/var   && \
	[ -d $(XBASEDIR)/dev  ] || mkdir -p $(XBASEDIR)/dev   && \
	[ -d $(XBASEDIR)/sys  ] || mkdir -p $(XBASEDIR)/sys   && \
	[ -d $(XBASEDIR)/mnt  ] || mkdir -p $(XBASEDIR)/mnt   && \
	[ -d $(XBASEDIR)/ui   ] || mkdir -p $(XBASEDIR)/ui    && \
	[ -d $(XBASEDIR)/ovr  ] || mkdir -p $(XBASEDIR)/ovr   && \
	[ -s $(XBASEDIR)/tmp  ] || ln -s /var/tmp  $(XBASEDIR)/tmp   && \
	[ -s $(XBASEDIR)/root ] || ln -s /var/root $(XBASEDIR)/root  && \
	[ ! -d $(XBASEDIR)/lib    ] || \rm -rf $(XBASEDIR)/lib       && \
	[ ! -d $(XBASEDIR)/lib64  ] || \rm -rf $(XBASEDIR)/lib64     && \
	[ ! -d $(XBASEDIR)/etc    ] || \rm -rf $(XBASEDIR)/etc       && \
	[ -d $(XBASEDIR)/lib    ] || mkdir -p $(XBASEDIR)/lib        && \
	[ -d $(XBASEDIR)/lib64  ] || mkdir -p $(XBASEDIR)/lib64      && \
	[ -d $(XBASEDIR)/etc    ] || mkdir -p $(XBASEDIR)/etc        && \
	[ -d $(XBASEDIR)/etc/init.d ] || mkdir -p $(XBASEDIR)/etc/init.d                   && \
	echo "ttyS2::respawn:/bin/sh"                   >  $(XBASEDIR)/etc/inittab         && \
	echo "::sysinit:/etc/init.d/rcS"                >> $(XBASEDIR)/etc/inittab         && \
	echo "::shutdown:/etc/init.d/rc.shutdown"       >> $(XBASEDIR)/etc/inittab         && \
	echo "\#!/bin/bash "                            >  $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount -t proc proc /proc "                >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount -t tmpfs ram /var -o size=4m "      >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount -t sysfs sys /sys "                 >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mkdir /var/tmp "                          >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mkdir /var/root "                         >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "export PATH=/bin:/sbin:/usr/bin:/usr/sbin">> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "export LD_LIBRARY_PATH=/lib:/lib64 "      >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount /dev/mmcblk0p5 /mnt"                >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount /dev/mmcblk0p6 /ui"                 >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "[ -d /mnt/usr  ] || ( echo \"External disk is not mounted !!\" ; exit 1 ) " \
							>> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "[ -d /mnt/work ] || ( echo \"Work disk is not mounted !!\" ; exit 1 ) " \
							>> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "[ -d /mnt/up   ] || ( echo \"Upper disk is not mounted !!\" ; exit 1 ) " \
							>> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "echo \"Mounting...\" "                    >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount -t overlay -o lowerdir=/disk:/ui:/mnt/usr,upperdir=/mnt/up,workdir=/mnt/work overlay /ovr" \
							>> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "echo \"Device file system\" "             >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "mount -t devtmpfs devfs /ovr/dev "        >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "echo \"Change root !!\" "                 >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "chroot /ovr /etc/rc.init "                >> $(XBASEDIR)/etc/init.d/rcS      && \
	echo "\#!/bin/bash"                             >  $(XBASEDIR)/etc/init.d/rc.shutdown   && \
	echo "root:x:0:"                                >  $(XBASEDIR)/etc/group           && \
	echo "root:x:0:0:root:/root:/bin/sh"            >  $(XBASEDIR)/etc/passwd          && \
	echo "root:*:0:0:99999:7:::"                    >  $(XBASEDIR)/etc/shadow          && \
	echo "Done" > /dev/null 
