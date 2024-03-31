SYSLIN=isolinux/build/syslinux

BOOTARGS="console=ttyS0 root=/dev/ram0 ro rootfstype=squashfs rootwait"

##
## Kernel version linux-6.x.y  --> Ramdisk size : 256Mbytes
ifneq (,$(findstring linux-6,$(KERNELVER)))
	BOOTARGS+=" ramdisk_size=262144"
endif

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
VMFILE=./vm-$(_CORE_).sh

run:
	@sudo $(VMFILE) run $(BDDIR)

stop:
	@sudo $(VMFILE) stop $(BDDIR)
