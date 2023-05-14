#!/bin/sh

if [ $# -lt  1 ]; then
	echo "disk device ? sde/sdd??? "
	exit 1
fi

boot=$11
part=$12

if [ `mount | grep /dev/$part | wc -l` -eq 0 ]; then
	echo "disk /dev/$part was not mounted."
	exit 1
fi

bootpoint=`mount  | grep $boot | awk '{print $3}'`
mntpoint=`mount  | grep $part | awk '{print $3}'`

[ ! -d $mntpoint ] || (
	sudo umount $mntpoint ; \
	sudo dd if=rootfs.squashfs of=/dev/$part bs=128k \
)

[ ! -d $bootpoint ] || (
	cp iso/boot/vmlinuz $bootpoint ; \
	cp iso/boot/u-boot.bin $bootpoint ; \
	sudo umount $bootpoint ; \
)

sync
sync
sync

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
