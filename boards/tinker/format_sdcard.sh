#!/bin/sh

LOGFILE=/tmp/linuxrpi.rpi3.format_disk.log
TMPDIR=/tmp/linuxrpi.rpi3.tmpdir

export BLOCKSZ=4M

DD=dd conv=sync bs=${BLOCKSZ}
LOADERFILE=iso/boot/idbloader.img
UBOOTFILE=iso/boot/u-boot.img

##
## Partition Size ...
##
export LOADERPSIZE=128M
BOOTPSIZE=128M
RAMDISKPSIZE=1G
ROOTPSIZE=16G
OVRPSIZE=8G
UIPSIZE=6G
CFGPSIZE=10M

##COUNT=`expr "${LOADERPSIZE/M/}" / "${BLOCKSZ/M/}"`
COUNT=32

##
## BOOT Partition
BOOTFILE=boot.tgz
BOOTPNAME=BOOT

##
## RAMDISK Partition 
RAMDISKFILE=rootfs.squashfs

##
## IMAGE Partition 
IMAGEFILE=image.ext4

##
## UI Partition 
UIFILE=ui.ext4

##
## CFG Partition
CFGVOLNM=config
CFGFILE=config.ext4

## who is running this script 
whoami=`whoami`
if [ "$whoami" != "root" ]; then 
       echo "Root user only can run this script... " 
       exit
fi

if [ $# -lt  1 ]; then
	echo "disk device ? sde/sdd??? "
	exit 1
fi

## which drive....
DRIVE=$1

read -p "Please enter 'yes' if you want to keep going on with /dev/$DRIVE disk : " answer
if [ "$answer" != "yes" ]; then 
	echo "Answer is not 'yes'"
	exit
fi

## Unmounting disks..
done=0
while [ $done != 1 ];
do
	res=`mount | grep $1 | head -n 1 | awk '{print $3}'`
	if [ "$res" != "" ]; then
		echo "Unmounting $res"
		sudo umount $res
	else
		done=1
	fi
	sleep 1
done

##
## Deleting existing partitions 
##
echo "Deleting partitions in /dev/$DRIVE "
sudo sh -c " fdisk --wipe-partitions always /dev/$DRIVE <<END
d 

d

d

d

d

d

d

d

d

d

d

d

w
END " &>> $LOGFILE

sleep 1
echo "-----------------------------------------------------"

##
## Making partitions 
##
echo "Making partitions in /dev/$DRIVE "
sudo sh -c " fdisk --wipe-partitions always /dev/$DRIVE <<END
n
p
1

+$LOADERPSIZE

n
p
2

+$BOOTPSIZE

n
p
3

+$RAMDISKPSIZE

n
e

+$ROOTPSIZE

n

+$OVRPSIZE

n

+$UIPSIZE

n

+$CFGPSIZE

w
END " &>> $LOGFILE

sleep 1
echo "-----------------------------------------------------"

echo "Changing BOOT PARTITION to FAT"
sudo sh -c " fdisk --wipe-partitions always /dev/$DRIVE <<END
t
2
c
w
END " &>> $LOGFILE

sleep 1
echo "-----------------------------------------------------"

echo "Building LOADER partition /dev/${DRIVE}1"
if [ -f ${LOADERFILE} -a -f ${UBOOTFILE} ]; then
	echo ""
    echo "Zeroing..."
	echo ""
	sudo ${DD} if=/dev/zero of=/dev/${DRIVE}1 count=${COUNT}

	echo ""
    echo "Wring IDBLOADER"
	echo ""
	sudo ${DD} if=${LOADERFILE} of=/dev/${DRIVE}1 seek=64

	echo ""
    echo "Wring BOOTLOADER"
	echo ""
	sudo ${DD} if=${UBOOTFILE}  of=/dev/${DRIVE}1 seek=16384
fi

echo "Building BOOT partition /dev/${DRIVE}2 "
if [ -f $BOOTFILE ]; then
	mkfs.vfat -n $BOOTPNAME /dev/${DRIVE}2 
	[ ! -d $TMPDIR ] || \rm -rf $TMPDIR
	mkdir -p $TMPDIR
	mount /dev/${DRIVE}2 $TMPDIR 
	tar zxvf $BOOTFILE -C $TMPDIR > /dev/null 2>&1 
	sync
	sync
	sync
	umount $TMPDIR
	\rm -rf $TMPDIR
fi

echo "Building RAMDISK partition /dev/${DRIVE}3 "
if [ -f $RAMDISKFILE ]; then
	sudo ${DD} if=$RAMDISKFILE of=/dev/${DRIVE}3 
fi

echo "Building IMAGE partition /dev/${DRIVE}5 - takes long time... "
if [ -f $IMAGEFILE ]; then
	sudo ${DD} if=$IMAGEFILE of=/dev/${DRIVE}5 
	sync
	sync
	sync
fi

echo "Building UI partition /dev/${DRIVE}6 - takes long time... "
if [ -f $UIFILE ]; then
	## sudo ${DD} if=$UIFILE of=/dev/${DRIVE}6 
	sync
	sync
	sync
fi

echo "Building CFG partition /dev/${DRIVE}7"
if [ -f $CFGFILE ]; then
	sudo ${DD} if=$CFGFILE of=/dev/${DRIVE}7 
	sync
	sync
	sync
fi

