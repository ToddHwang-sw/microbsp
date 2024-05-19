BOOTIMAGE=boot.ext2

BOOTARGS="console=ttyS0,115200 root=/dev/ram0 ro rootfstype=squashfs rootwait"

prepare:
	@[ -d $(isodir) ]        || mkdir -p $(isodir)
	@[ -d $(isodir)/boot ]   || mkdir -p $(isodir)/boot 
	@[ -d $(isodir)/grub ]   || mkdir -p $(isodir)/grub
	@make -C grub download prepare all

all:
	@echo "Image nothing to be compiled"

install:
	@[ ! -d image ] || ( \
		cd image; \
		[ ! -f ./setup.sh ] || \
			sudo ./setup.sh ../$(BOOTIMAGE) $(subst linux,vmlinuz,$(KERNELVER)) "$(BOOTARGS)" )

clean uninstall:
	@\rm -rf $(isoname)
	@\rm -rf $(isodir)

#########################################################################################################################
#
#
#  VM execution ... 
#
#
#########################################################################################################################
run:

stop:
