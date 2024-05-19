#!/bin/sh

LOGFILE=/tmp/linuxrpi.grub.format_disk.log

DDCMD=dd conv=sync bs=2M

##
## Partition Size ...
##
BIOSPSIZE=4G
CFGPSIZE=10M
EXT4SIZE=10G

##
## BOOT Partition
BOOTFILE=boot.ext2

##
## Temporary for installing grub 
TMPFOLDER=./__grub_install_tmp

##
## IMAGE Partition 
IMAGEFILE=image.ext4

##
## CFG Partition
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
## Destroying GPT/MBR ... 
##
echo "Destroying partitions in /dev/$DRIVE "
sudo dd if=/dev/zero of=/dev/$DRIVE bs=1M count=32

##
## Setting GPT table
##
echo "Creating GPT in /dev/$DRIVE "
sudo sh -c " gdisk /dev/$DRIVE <<END
2
2
w
y
END " &>> $LOGFILE

sync
sync
sync

echo "-----------------------------------------------------"
sleep 5

##
## Making partitions 
##
echo "Making partitions in /dev/$DRIVE "
sudo sh -c " fdisk --wipe-partitions always /dev/$DRIVE <<END
n


+$BIOSPSIZE
n


+$CFGPSIZE
n


+$EXT4SIZE
w
END " &>> $LOGFILE

sync
sync
sync

echo "-----------------------------------------------------"
sleep 2

echo "Building BOOT partition /dev/${DRIVE}1"
if [ -f ${BOOTFILE} ]; then
	sudo ${DDCMD} if=${BOOTFILE} of=/dev/${DRIVE}1
	sync
	sync
	sync
fi

sync
sync
sync
sleep 1
echo "-----------------------------------------------------"

echo "Building CONFIG partition /dev/${DRIVE}2 "
if [ -f ${CFGFILE} ]; then
	sudo ${DDCMD} if=${CFGFILE} of=/dev/${DRIVE}2
	sync
	sync
	sync
fi

sync
sync
sync
sleep 1
echo "-----------------------------------------------------"

echo "Building IMAGE partition /dev/${DRIVE}3 - takes long time... "
if [ -f $IMAGEFILE ]; then
	sudo ${DDCMD} if=$IMAGEFILE  of=/dev/${DRIVE}3
	sync
	sync
	sync
fi

sync
sync
sync
sleep 1
echo "-----------------------------------------------------"

echo ""
echo "GRUB installation into /dev/${DRIVE}1"
echo ""
[ ! -f ${TMPFOLDER} ] || sudo \rm -rf ${TMPFOLDER}
sudo mkdir -p ${TMPFOLDER}
echo ""
echo "Mount  /dev/${DRIVE}1"
echo ""
sudo mount -t ext2 /dev/${DRIVE}1 ${TMPFOLDER}
echo ""
echo "Grub-install"
echo ""
sudo grub-install --target=i386-pc --boot-directory=${TMPFOLDER}/boot /dev/${DRIVE}
echo ""
echo "Unmounting /dev/${DRIVE}1"
echo ""
sudo umount ${TMPFOLDER}
sudo \rm -rf ${TMPFOLDER}

