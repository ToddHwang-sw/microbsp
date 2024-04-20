FILENM=$(KERNELVER).tar.xz
LOC=https://www.kernel.org/pub/linux/kernel/v5.x/$(FILENM)

export LINUXMAKE=$(KERNELMAKE) -j 4 -C .

## EXT4/Squash FS image ..
ROOTFSIMAGEVOL=rootfs

ifeq ("$(SQUASHROOTFS)","y")
	ROOTFSIMAGEFN=$(ROOTFSIMAGEVOL).squashfs
else
	ROOTFSIMAGEFN=$(ROOTFSIMAGEVOL).ext4
endif

ROOTFSIMAGE=$(ROOTFSIMAGEFN)
ROOTFSIMAGEBLKS=500

EXTMDIRS=

##
## Files installed from linux kernel 
##
KERNEL_FILES=vmlinuz System.map config

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		[ -f source/$(FILENM) ] || ( cd source; wget $(LOC) ) )
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || (                          \
		tar xvf source/$(FILENM) -C $(BUILDDIR) ;                  \
		cd $(BUILDDIR)/$(KERNELVER);                               \
		cat ../../../patch/patch.$(KERNELVER) | patch -p 1;        \
		cp  ../../../patch/$(KERNELVER).config .config;            \
		make ARCH=arm olddefconfig )

all:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER) && \
		[ ! -f ../build.log ] || rm ../build.log && \
		touch ../build.log && \
		$(LINUXMAKE) zImage       2>&1  | tee -a ../build.log  &&    \
		$(LINUXMAKE) modules      2>&1  | tee -a ../build.log  &&    \
		$(LINUXMAKE) dtbs         2>&1  | tee -a ../build.log  &&    \
		$(LINUXMAKE) INSTALL_MOD_PATH=$(destination) modules_install 2>&1  | tee -a ../build.log \
	)
	@find $(destination)/lib/ -type f -name "*.ko" -print | \
		sh $(TOPDIR)/scripts/modules-alias.sh $(destination)/etc/modules.conf

extmod:
	@for dir in $(EXTMDIRS); do \
		[ ! -f modules/$$dir/build.log ] || rm modules/$$dir/build.log && \
		touch modules/$$dir/build.log  && \
		make KERNSRC=$(KERNDIR)/$(KERNELVER) -C modules/$$dir prepare all install  2>&1  | tee -a modules/$$dir/build.log ; \
	done

clean:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) clean )

dnfw:

install: 
	[ ! -f $(BDDIR)/$(ROOTFSIMAGEFN) ] || \rm -rf $(BDDIR)/$(ROOTFSIMAGEFN)
	[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		if [ "$(SQUASHROOTFS)" = "y" ]; then \
			mksquashfs $(destination) $(BDDIR)/$(ROOTFSIMAGE) -force-uid 0 -force-gid 0 ; \
		else                                                                               \
			$(TOPDIR)/scripts/setupdisk.sh build $(destination) $(ROOTFSIMAGEVOL) $(BDDIR)/$(ROOTFSIMAGE) $(ROOTFSIMAGEBLKS) ; \
		fi )
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) zImage)
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		\rm -rf $(isodir)/kernel/*-$(subst linux-,,$(KERNELVER)) ; \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) INSTALL_PATH=$(isodir)/kernel install; \
		cp arch/arm/boot/zImage $(isodir)/kernel;           \
		$(LINUXMAKE) INSTALL_DTBS_PATH=$(isodir)/kernel dtbs_install )
	@[ ! -f $(isodir)/kernel/zImage ] || (            \
		cd $(isodir)/kernel; \
		tar zcvf ../../$(BDTARGZ) zImage rk3288-tinker.dtb > /dev/null  \
		)

uninstall:
	@[ ! -f ../$(ROOTFSIMAGEFN) ] || \rm -rf ../$(ROOTFSIMAGEFN)

config:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); $(LINUXMAKE) menuconfig )

