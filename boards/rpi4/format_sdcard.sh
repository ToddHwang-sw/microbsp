#!/bin/sh

LOGFILE=/tmp/linuxrpi.rpi4.format_disk.log
TMPDIR=/tmp/linuxrpi.rpi4.tmpdir

##
## Partition Size ...
##
BOOTPSIZE=128M
RAMDISKPSIZE=1G
ROOTPSIZE=8G
OVRPSIZE=8G
UIPSIZE=4G
CFGPSIZE=10M

## sudo dd command 
DDCMD="sudo dd bs=128M conv=fsync status=progress "

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

+$BOOTPSIZE

n
p
2

+$RAMDISKPSIZE

n
p
3

+$ROOTPSIZE

n
e

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
1
c
w
END " &>> $LOGFILE

sleep 1
echo "-----------------------------------------------------"

echo "Turning on Bootable Flag"
sudo sh -c " fdisk --wipe-partitions always /dev/$DRIVE <<END
a
1
w
END " &>> $LOGFILE

echo "Building BOOT partition /dev/${DRIVE}1"
if [ -f $BOOTFILE ]; then
    echo "Yes Boot FILe!!!"
	mkfs.vfat -n $BOOTPNAME /dev/${DRIVE}1 
	[ ! -d $TMPDIR ] || \rm -rf $TMPDIR
	mkdir -p $TMPDIR
	mount /dev/${DRIVE}1 $TMPDIR 
	tar zxvf $BOOTFILE -C $TMPDIR > /dev/null 2>&1 
	sync
	sync
	sync
	umount $TMPDIR
	\rm -rf $TMPDIR
fi

echo "Building RAMDISK partition /dev/${DRIVE}2 "
if [ -f $RAMDISKFILE ]; then
	$DDCMD if=$RAMDISKFILE of=/dev/${DRIVE}2
	sync
	sync
	sync
fi

echo "Building IMAGE partition /dev/${DRIVE}3 - takes long time... "
if [ -f $IMAGEFILE ]; then
	$DDCMD if=$IMAGEFILE  of=/dev/${DRIVE}3
	sync
	sync
	sync
fi

echo "Building UI partition /dev/${DRIVE}5 - takes long time... "
if [ -f $UIFILE ]; then
	$DDCMD if=$UIFILE  of=/dev/${DRIVE}5
	sync
	sync
	sync
fi

echo "Building CFG partition /dev/${DRIVE}6"
if [ -f $CFGFILE ]; then
	$DDCMD if=$CFGFILE  of=/dev/${DRIVE}6
	sync
	sync
	sync
fi

