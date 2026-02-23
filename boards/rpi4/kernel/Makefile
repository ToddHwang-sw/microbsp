## Naming rule of Raspberry PI kernel sources
BRANCH=$(subst linux-,,$(KERNELVER))

FILENM=$(BRANCH).zip
LOC=https://github.com/raspberrypi/linux/archive/refs/heads/$(FILENM)

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

WGETCMD=wget --no-check-certificate

## Check EMBEDDED_INITRD is used now ???
RPI_KVER=$(shell cat $(BUILDDIR)/$(KERNELVER)/include/config/kernel.release)

##
## Files installed from linux kernel 
##
KERNEL_FILES=vmlinuz System.map config

##
## External modules
##
EXTMDIRS=

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		[ -f source/$(FILENM) ] || ( cd source; $(WGETCMD) $(LOC) ) )
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || (                          \
		unzip source/$(FILENM) -d $(BUILDDIR) ;                    \
		cd $(BUILDDIR)/$(KERNELVER);                               \
		cp ../../../patch/$(KERNELVER).config .config;             \
		$(LINUXMAKE) olddefconfig )

all:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER) && \
		[ ! -f ../build.log ] || rm ../build.log && \
		touch ../build.log && \
		$(LINUXMAKE) Image.gz     2>&1  | tee -a ../build.log  &&    \
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
	@[ ! -f $(BDDIR)/$(ROOTFSIMAGEFN) ] || \rm -rf $(BDDIR)/$(ROOTFSIMAGEFN)
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		if [ "$(SQUASHROOTFS)" = "y" ]; then \
			mksquashfs $(destination) $(BDDIR)/$(ROOTFSIMAGE) -force-uid 0 -force-gid 0 ; \
		else                                                                               \
			$(TOPDIR)/scripts/setupdisk.sh build $(destination) $(ROOTFSIMAGEVOL) $(BDDIR)/$(ROOTFSIMAGE) $(ROOTFSIMAGEBLKS) ; \
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

uninstall:
	@[ ! -f ../$(ROOTFSIMAGEFN) ] || \rm -rf ../$(ROOTFSIMAGEFN)

config:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); $(LINUXMAKE) menuconfig )

