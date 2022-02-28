FILENM=master.zip
LOC=https://github.com/Xilinx/linux-xlnx/archive/$(FILENM)

export LINUXMAKE=$(KERNELMAKE) -j 4 -C .

## Squash FS image ..
RAMFSIMAGEFN=ramdisk.squash
RAMFSIMAGE=arch/arm64/ramdisk/$(RAMFSIMAGEFN)

## Check EMBEDDED_INITRD is used now ???
USING_EMBEDDED_INITRD=$(shell cat $(BUILDDIR)/$(KERNELVER)/include/config/auto.conf | grep CONFIG_EMBEDDED_INITRD | wc -l)
RPI_KVER=$(shell cat $(BUILDDIR)/$(KERNELVER)/include/config/kernel.release)

##
## Files installed from linux kernel 
##
KERNEL_FILES=vmlinuz System.map config

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		[ -f source/$(FILENM) ] || ( cd source; wget $(LOC) ) )
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || (                          \
		$(UNCOMPRESS) source/$(FILENM) $(BUILDDIR) ;               \
		cd $(BUILDDIR)/$(KERNELVER);                               \
		make ARCH=arm64 xilinx_zynqmp_defconfig )

all:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) Image.gz ; \
		$(LINUXMAKE) modules ; \
		$(LINUXMAKE) dtbs ;    \
		$(LINUXMAKE) INSTALL_MOD_PATH=$(destination) modules_install )
	@find $(destination)/lib/ -type f -name "*.ko" -print | \
		sh $(TOPDIR)/scripts/modules-alias.sh $(destination)/etc/modules.conf

extmod:

dnfw:

clean:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) clean )

install: 
	@if [ "$(USING_EMBEDDED_INITRD)" = "1" ]; then                      \
		[ ! -f $(BUILDDIR)/$(KERNELVER)/$(RAMFSIMAGE) ] || (       \
			\rm -rf $(BUILDDIR)/$(KERNELVER)/$(RAMFSIMAGE) );  \
	else                                                               \
		[ ! -f $(BDDIR)/$(RAMFSIMAGEFN) ] || (                     \
			\rm -rf $(BDDIR)/$(RAMFSIMAGEFN) );                \
	fi
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		if [ "$(USING_EMBEDDED_INITRD)" = "1" ]; then  \
			mksquashfs $(destination) $(BUILDDIR)/$(KERNELVER)/$(RAMFSIMAGE) -force-uid 0 -force-gid 0 ; \
		else                                                                                 \
			mksquashfs $(destination) $(BDDIR)/$(RAMFSIMAGEFN) -force-uid 0 -force-gid 0 ; \
		fi )	
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) Image.gz )
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		\rm -rf $(isodir)/boot/*-$(subst linux-,,$(KERNELVER)) ; \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) INSTALL_PATH=$(isodir)/boot install; \
		$(LINUXMAKE) INSTALL_DTBS_PATH=$(isodir)/boot dtbs_install )
	@echo ""
	@echo "Name changing..."
	@echo ""
	@for f in $(KERNEL_FILES); do \
		[ ! -f $(isodir)/boot/$$f-$(RPI_KVER) ] ||  ( \
			mv $(isodir)/boot/$$f-$(RPI_KVER) $(isodir)/boot/$$f ) ; \
	done

config:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); $(LINUXMAKE) menuconfig )

