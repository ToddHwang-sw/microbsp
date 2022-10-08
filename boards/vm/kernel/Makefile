LOC=https://www.kernel.org/pub/linux/kernel/v5.x/
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

##
## USB hotplug...
##
HOTPLUGFWTMPDIR=/tmp/___hotplug_fw_temp_dir__
HOTPLUGFWFILE=hotplug-2004_03_29
HOTPLUGFWURL=https://sourceforge.net/projects/linux-hotplug/files/hotplug/2004_03_29/$(HOTPLUGFWFILE).tar.gz

##
## Check configuration "CONFIG_EMBEDDED_INITRD" 
##
USE_EMBEDDED_INITRD=`cat $(BUILDDIR)/$(KERNELVER)/.config | grep CONFIG_EMBEDDED_INITRD=y | wc -l`

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -f source/$(FILENM) ] || wget -P source $(LOC)/$(FILENM);
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		tar zxvf source/$(FILENM) -C $(BUILDDIR) ;                       \
		cp patch/patch                $(BUILDDIR)/$(KERNELVER)/;         \
		cp patch/$(KERNELVER).config  $(BUILDDIR)/$(KERNELVER)/.config ; \
		cd $(BUILDDIR)/$(KERNELVER) ;                                    \
		cat patch | patch -p 1 ;                                         \
		make ARCH=$(_CORE_) oldconfig \
	)

all:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) bzImage ; \
		$(LINUXMAKE) modules ; \
		$(LINUXMAKE) INSTALL_MOD_PATH=$(destination) modules_install )
	@find $(destination)/lib/ -type f -name "*.ko" -print | \
		sh $(TOPDIR)/scripts/modules-alias.sh $(destination)/etc/modules.conf

extmod:
	@for dir in $(EXTMDIRS); do \
		make KERNSRC=$(KERNDIR)/$(KERNELVER) -C modules/$$dir prepare all install; \
	done

clean uninstall:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) clean )

dnfw:
	@( \
		[ ! -d $(HOTPLUGFWTMPDIR) ] || \rm -rf $(HOTPLUGFWTMPDIR);   \
		mkdir -p $(HOTPLUGFWTMPDIR) ;                                \
		mkdir -p $(HOTPLUGFWTMPDIR)/INSTALL ;                        \
		wget  -P $(HOTPLUGFWTMPDIR) $(HOTPLUGFWURL);                 \
		tar zxvf $(HOTPLUGFWTMPDIR)/$(HOTPLUGFWFILE).tar.gz -C $(HOTPLUGFWTMPDIR)/INSTALL ;    \
		cp -rf $(HOTPLUGFWTMPDIR)/INSTALL/$(HOTPLUGFWFILE)/etc/*  $(XBASEDIR)/etc ;            \
		cp -rf $(HOTPLUGFWTMPDIR)/INSTALL/$(HOTPLUGFWFILE)/sbin/* $(XBASEDIR)/sbin ;           \
	)

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
		$(LINUXMAKE) bzImage )
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		\rm -rf $(isodir)/boot/*-$(subst linux-,,$(KERNELVER)) ; \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) INSTALL_PATH=$(isodir)/boot install )
	
config:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); $(LINUXMAKE) menuconfig )

