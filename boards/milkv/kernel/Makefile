## Source code location
LOC=https://www.kernel.org/pub/linux/kernel/v6.x/

FILENM=$(KERNELVER).tar.gz

LINUXMAKE=$(KERNELMAKE) -j 8 -C .

## Squash FS image ..
ROOTFSIMAGEVOL=rootfs

ifeq ("$(SQUASHROOTFS)","y")
	ROOTFSIMAGEFN=$(ROOTFSIMAGEVOL).squashfs
else
	ROOTFSIMAGEFN=$(ROOTFSIMAGEVOL).ext4
endif

ROOTFSIMAGE=$(ROOTFSIMAGEFN)
ROOTFSIMAGEBLKS=500

WGETCMD=wget --no-check-certificate

##
## Check configuration "CONFIG_EMBEDDED_INITRD" 
##
USE_EMBEDDED_INITRD=`cat $(BUILDDIR)/$(KERNELVER)/.config | grep CONFIG_EMBEDDED_INITRD=y | wc -l`

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -f source/$(FILENM) ] || $(WGETCMD) -P source $(LOC)/$(FILENM);
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		tar zxvf source/$(FILENM) -C $(BUILDDIR) ;                       \
		cp patch/$(KERNELVER).patch   $(BUILDDIR)/$(KERNELVER)/;         \
		cp patch/$(KERNELVER).config  $(BUILDDIR)/$(KERNELVER)/.config ; \
		cd $(BUILDDIR)/$(KERNELVER) ;                                    \
		cat $(KERNELVER).patch | patch -p 1 ;                            \
		$(LINUXMAKE) olddefconfig \
	)

all:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) Image.gz ; \
		$(LINUXMAKE) modules ; \
		$(LINUXMAKE) INSTALL_MOD_PATH=$(destination) modules_install )
	@find $(destination)/lib/ -type f -name "*.ko" -print | \
		sh $(TOPDIR)/scripts/modules-alias.sh $(destination)/etc/modules.conf

extmod:
	@for dir in $(EXTMDIRS); do \
		$(LINUXMAKE) KERNSRC=$(KERNDIR)/$(KERNELVER) -C modules/$$dir prepare all install; \
	done

clean uninstall:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) clean )

dnfw:

install: 
	@[ ! -f $(BDDIR)/$(ROOTFSIMAGEFN) ] || \rm -rf $(BDDIR)/$(ROOTFSIMAGEFN)
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		if [ "$(SQUASHROOTFS)" = "y" ]; then \
			mksquashfs $(destination) $(BDDIR)/$(ROOTFSIMAGE) -force-uid 0 -force-gid 0 ; \
		else                                                                               \
			$(TOPDIR)/scripts/setupdisk.sh build $(destination) $(ROOTFSIMAGEVOL) $(BDDIR)/$(ROOTFSIMAGE) $(ROOTFSIMAGEBLKS) ; \
		fi )
	@if [ "$(USE_EMBEDDED_INITRD)" = "1" ]; then \
		[ ! -d $(BUILDDIR)/$(KERNELVER)/arch/x86/ramdisk ]  || \
			cp $(BDDIR)/$(ROOTFSIMAGE) $(BUILDDIR)/$(KERNELVER)/arch/x86/ramdisk ; \
	fi
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) Image.gz )
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		\rm -rf $(isodir)/boot/*-$(subst linux-,,$(KERNELVER)) ; \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) INSTALL_PATH=$(isodir)/boot install )
	
config:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); $(LINUXMAKE) menuconfig )

