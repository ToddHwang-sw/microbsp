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

EXTMDIRS=rtl8192eu

##
## Files installed from linux kernel 
##
KERNEL_FILES=vmlinuz System.map config

##
## Regulatory FWs
##
CRDAFWLOC=git://git.kernel.org/pub/scm/linux/kernel/git/sforshee/wireless-regdb.git
CRDAFW0S=\
	regulatory.bin \
	sforshee.x509.pem \
	sforshee.key.pub.pem
CRDAFW1S=\
	regulatory.db \
	regulatory.db.p7s
CRDAFWTMPDIR=\
	/tmp/___tinker_linuxswtree_linux-crda_firmware____
CRDATMPTMP=\
	/tmp/___tinker_linuxswtree_linux-crda_signature___

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
	@[ -d $(BDDIR)/rootfs/lib/crda ] || mkdir -p $(BDDIR)/rootfs/lib/crda
	@for f in $(CRDAFW0S); do \
		[ -f $(BDDIR)/rootfs/lib/crda/$$f ] || ( \
			[ -d $(CRDAFWTMPDIR) ] || ( \
				echo "" ;                                 \
				echo "Copyiing Linux-Firmware from $(CRDAFWLOC)" ; \
				echo "" ;                                \
				mkdir -p $(CRDAFWTMPDIR);                \
				$(eval __CURDIR:=${PWD})		         \
				cd $(CRDAFWTMPDIR);                      \
				git clone --single-branch $(CRDAFWLOC) ; \
				cd $(CRDAFWTMPDIR)/wireless-regdb;       \
				git rev-list -n 1 --until=\"$(shell cat $(TOPDIR)/date.inc)\" master > $(CRDATMPTMP) ; \
				git checkout -q `cat $(CRDATMPTMP)`;     \
				\rm -rf $(CRDATMPTMP);                   \
				cd $(__CURDIR);                          \
				cd $(BDDIR)/kernel                       \
			) ;                                          \
			cp -rf $(CRDAFWTMPDIR)/wireless-regdb/$$f $(BDDIR)/rootfs/lib/crda/$$f \
		) ; \
	done
	@[ -d $(BDDIR)/rootfs/lib/firmware ] || mkdir -p $(BDDIR)/rootfs/lib/firmware
	@for f in $(CRDAFW1S); do \
		[ -f $(BDDIR)/rootfs/lib/firmware/$$f ] || ( \
			[ -d $(CRDAFWTMPDIR) ] || ( \
				echo "" ;                                 \
				echo "Copying Linux-Firmware from $(CRDAFWLOC)" ; \
				echo "" ;                                \
				mkdir -p $(CRDAFWTMPDIR);                \
				$(eval __CURDIR:=${PWD})		         \
				cd $(CRDAFWTMPDIR);                      \
				git clone --single-branch $(CRDAFWLOC);  \
				cd $(CRDAFWTMPDIR)/wireless-regdb;       \
				git rev-list -n 1 --until=\"$(shell cat $(TOPDIR)/date.inc)\" master > $(CRDATMPTMP) ; \
				git checkout -q `cat $(CRDATMPTMP)`;     \
				\rm -rf $(CRDATMPTMP);                   \
				cd $(__CURDIR);                          \
				cd $(BDDIR)/kernel                       \
			) ;                                          \
			cp -rf $(CRDAFWTMPDIR)/wireless-regdb/$$f $(BDDIR)/rootfs/lib/firmware/$$f \
		) ; \
	done
	@\rm -rf $(CRDAFWTMPDIR)

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

