#!/bin/bash

TMPDIR=/tmp/microbsp/vm-grub-dir
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
E2DISKFN=$1
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
[ -d $DISK/$GRUBDIR ] || mkdir -p $DISK/$GRUBDIR

##
## Wriuting grub.cfg
if [ ! -f $DISK/$GRUBDIR/grub.cfg ]; then
	touch $DISK/$GRUBDIR/grub.cfg
	echo "GRUB_TERMINAL=serial"                   >> $DISK/$GRUBDIR/grub.cfg
	echo "GRUB_SERIAL_COMMAND=\"serial --speed=115200 --unit=0 --word=8 --parity=no --stop=1\""  >> $DISK/$GRUBDIR/grub.cfg
	echo ""                                       >> $DISK/$GRUBDIR/grub.cfg
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
sudo dd if=/dev/zero of=$E2DISKFN bs=4k count=50k
sudo mkfs.ext2 -L boot $E2DISKFN
sudo mount -o loop $E2DISKFN $E2MNTPT
sudo cp -rf $DISK/* $E2MNTPT/
sync
sync
sync
sync
sudo umount $E2MNTPT
sync
sync
sync

