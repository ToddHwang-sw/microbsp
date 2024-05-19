## H/W core 
export _CORE_=aarch64

## architecture name 
export _ARCH_=$(_CORE_)-xilinx

## platfom string ...
export PLATFORM=$(_CORE_)-unknown-linux

## kernel version 
export KERNELVER=linux-xlnx-master

##
## folders to be included
##
export EXTDIR+=\
			tcpdump

##
## external disk for editable parttition 
##
export EXTDISK=

##
## Mounting commands & root change command 
##
export MOUNTCMD=mount /dev/mmcblk0p3 /mnt
export SETROOT=chroot /ovr /etc/rc.init
