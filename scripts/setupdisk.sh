#!/bin/sh

MENU=$1
NDIR=/tmp/___dumpdir_123AbCdEfGh

case "$MENU" in
	"build")
		SRCDIR=$2
		VOL=$3
		FILENM=$4
		COUNT=$5

		##
		## bs x count = G bytes image
		##
		TRANSFER="bs=1M count=$COUNT"

		[ -d $SRCDIR ] || ( echo "$SRCDIR does not exist"; exit1 )
		[ -d $NDIR ] || ( \
			echo "Making folders for overlayFS"; \
			mkdir -p $NDIR/work )

		## Creating 2G image
		echo "Running... dd if=/dev/zero of=$FILENM $TRANSFER"
		dd if=/dev/zero of=$FILENM $TRANSFER
		echo "Runnug mkfs.ext4 $FILENM"

		if [ "$VOL" = "" ]; then 
			VOL="ext"
		fi
		mkfs.ext4 -L $VOL $FILENM

		echo "Running mount -o loop -t ext4 $FILENM $NDIR"
		sudo mount -o loop -t ext4 $FILENM $NDIR
		cd $SRCDIR; \
			sudo find . -print -depth | sudo cpio -pdm $NDIR
		sync
		sync
		sync
		echo "Running sudo umount $NDIR"
		sudo umount $NDIR
		\rm -rf $NDIR
		;;

	"clean")
		FILENM=$2

		[ ! -f $FILENM ] || \rm -rf $FILENM 
		;;
esac

