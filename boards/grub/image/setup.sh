#!/bin/bash

TMPDIR=/tmp/microbsp/vm-grub-dir
TMPFN=$TMPDIR/grub-rescue.iso
BOOTD=boot
GRUBDIR=$BOOTD/grub
NEED_MODULE=$GRUBDIR/i386-pc
GRUB_APP=grub-mkrescue

##
## Disks... 
DISK=disk
MNTPT=$TMPDIR/__vm_grub_mntpt

##
## ext2 disk 
E2TMPFN=$1
E2MNTPT=$TMPDIR/__vm_grub_ext2_mntpt

KERNEL=$2
PARAM="${*:3}"

##
## check param
if [ "$1" = "" -o "$2" = "" -o "$3" = "" ]; then
	echo "$0 <ext2 image> <kernel image> <booting command line>"
	exit 1
fi

##
## check 
[ `which $GRUB_APP` != 0 ] || ( echo "$GRUB_APP is not installed !!" ; exit 1 )

##
## construc temporary file 
[ ! -f $TMPFN ] || \rm -rf $TMPFN
[ -d $TMPDIR  ] || mkdir -p $TMPDIR

##
## module installation for booting
if [ ! -d $DISK/$NEED_MODULE ]; then

	##
	## rescue image creation 
	sudo $GRUB_APP -o $TMPFN
	[ -f $TMPFN ] || ( echo "$TMPFN creation failed" ; exit 1 )

	[ -d $MNTPT ] || mkdir -p $MNTPT

	##
	## mount rescue disk 
	sudo mount -o loop $TMPFN $MNTPT 
	[ -d $MNTPT/$NEED_MODULE ] || ( \
		echo "$TMPFN/$NEED_MODULE is not found!!" ; \
		sudo umount $MNTPT ; \
		exit 1 )

	##
	## Creating disk 
	mkdir -p $DISK/$NEED_MODULE

	##
	## copying module .. 
	sudo cp -rf $MNTPT/$NEED_MODULE/* $DISK/$NEED_MODULE

	sudo umount $MNTPT
fi


##
## Wriuting grub.cfg
if [ ! -f $DISK/$GRUBDIR/grub.cfg ]; then
	touch $DISK/$GRUBDIR/grub.cfg
	echo "set default=0"                          >> $DISK/$GRUBDIR/grub.cfg
	echo "set timeout=10"                         >> $DISK/$GRUBDIR/grub.cfg
	echo ""                                       >> $DISK/$GRUBDIR/grub.cfg
	echo "menuentry 'MBSP Linux' {"               >> $DISK/$GRUBDIR/grub.cfg
	echo "      echo   'Loading Linux' "          >> $DISK/$GRUBDIR/grub.cfg
	echo "      linux	/boot/$KERNEL $PARAM "    >> $DISK/$GRUBDIR/grub.cfg
	echo "}"                                      >> $DISK/$GRUBDIR/grub.cfg
fi

##
## Copying Kernel Images....
sudo cp -fr ../iso/boot/$KERNEL $DISK/$BOOTD/

## mount point ...
[ -d $E2MNTPT ] || mkdir -p $E2MNTPT

##
## EXT2 image creation 
sudo dd if=/dev/zero of=$E2TMPFN bs=4k count=100k
sudo mkfs.ext2 -L boot $E2TMPFN
sudo mount -o loop $E2TMPFN $E2MNTPT
sudo cp -rf $DISK/* $E2MNTPT/
sync
sync
sync
sync
sudo umount $E2MNTPT

