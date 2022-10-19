SYSLIN=isolinux/build/syslinux

BOOTARGS="console=ttyS0 root=/dev/ram0 ro rootfstype=squashfs rootwait"

QEMU=qemu-system-$(_CORE_)

VMMAC=00:0a:0b:0c:0d:0e

##
## QEMU configuration 
##
QEMU_RUN=\
		$(QEMU) -m 4096 -nographic -smp 4 \
		-device e1000,netdev=net0,mac=$(VMMAC) \
		-netdev tap,id=net0,script=$(BDDIR)/qemu-if.script \
		-cdrom $(isoname) \
		-drive file=$(EXT4HDD) \
		-drive file=$(EXT4CFG)

prepare:
	@[ -d $(isodir) ]          || mkdir -p $(isodir)
	@[ -d $(isodir)/boot ]     || mkdir -p $(isodir)/boot 
	@[ -d $(isodir)/isolinux ] || mkdir -p $(isodir)/isolinux
	@make -C isolinux prepare all
	@cp -f $(SYSLIN)/bios/core/isolinux.bin                 $(isodir)/isolinux
	@cp -f $(SYSLIN)/bios/com32/elflink/ldlinux/ldlinux.c32 $(isodir)/isolinux
	@echo "DEFAULT linux"                  > $(isodir)/isolinux/syslinux.cfg
	@echo " SAY Booting linux kernel...." >> $(isodir)/isolinux/syslinux.cfg
	@echo "LABEL linux"                   >> $(isodir)/isolinux/syslinux.cfg
	@echo " KERNEL /boot/$(subst linux,vmlinuz,$(KERNELVER))" >> $(isodir)/isolinux/syslinux.cfg
	@echo " APPEND $(BOOTARGS)"           >> $(isodir)/isolinux/syslinux.cfg
	@\rm -rf $(isodir)/boot/*

all:
	@echo "Image nothing to be compiled"

install:
	@mkisofs -o $(isoname) \
		-b isolinux/isolinux.bin -c isolinux/boot.cat \
		-no-emul-boot \
		-input-charset utf8 \
		-boot-load-size 4 \
		-boot-info-table -J -r \
		$(isodir)/

clean uninstall:
	@\rm -rf $(isoname)
	@\rm -rf $(isodir)


#########################################################################################################################
#
#
#  x86_64 VM execution ... 
#
#
#########################################################################################################################

run:
	@sudo $(QEMU_RUN)

stop:
	@sudo killall $(QEMU)

