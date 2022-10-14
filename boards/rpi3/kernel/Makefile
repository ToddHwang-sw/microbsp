## Naming rule of Raspberry PI kernel sources
BRANCH=$(subst linux-,,$(KERNELVER))

FILENM=$(BRANCH).zip
LOC=https://github.com/raspberrypi/linux/archive/$(FILENM)

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

## Check EMBEDDED_INITRD is used now ???
RPI_KVER=$(shell cat $(BUILDDIR)/$(KERNELVER)/include/config/kernel.release)

##
## Files installed from linux kernel 
##
KERNEL_FILES=vmlinuz System.map config

##
## External modules
##
EXTMDIRS=rtl8192eu

##
## Broadcom WiFi FWs required.... 
##
##    https://www.linuxquestions.org/questions/slackware-arm-108/raspberry-pi-3-b-wifi-nic-not-found-4175627137/
      ##
BRCMFWFN=firmware-brcm80211_20161130-3+rpt3_all.deb
BRCMFWLOC=https://archive.raspberrypi.org/debian/pool/main/f/firmware-nonfree/$(BRCMFWFN)
BRCMFWS=\
	brcm/bcm43xx-0.fw \
	brcm/bcm43xx_hdr-0.fw \
	brcm/brcmfmac43455-sdio.bin \
	brcm/brcmfmac43455-sdio.txt \
	brcm/brcmfmac43455-sdio.clm_blob
BRCMFWTMPDIR=\
	/tmp/___rpi_linuxswtree_linux-broadcom_firmware___

##
## Realtek WiFi FWs required....
##
RTLFWLOC=git://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git 
RTLFWTAG=tags/20191215
RTLFWS=\
	rtlwifi/rtl8192eu_ap_wowlan.bin \
	rtlwifi/rtl8192eu_nic.bin \
	rtlwifi/rtl8192eu_wowlan.bin
RTLFWTMPDIR=\
	/tmp/___rpi_linuxswtree_linux-realtek_firmware____

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
	/tmp/___rpi_linuxswtree_linux-crda_firmware____

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		[ -f source/$(FILENM) ] || ( cd source; wget $(LOC) ) )
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || (                          \
		unzip source/$(FILENM) -d $(BUILDDIR) ;                    \
		cd $(BUILDDIR)/$(KERNELVER);                               \
		cat ../../../patch/patch-$(BRANCH) | patch -p 1;           \
		cp ../../../patch/$(KERNELVER).config .config;             \
		make ARCH=arm64 oldconfig )

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
	@for dir in $(EXTMDIRS); do \
		make KERNSRC=$(KERNDIR)/$(KERNELVER) -C modules/$$dir prepare all install; \
	done

clean:
	@[ ! -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		cd $(BUILDDIR)/$(KERNELVER); \
		$(LINUXMAKE) clean )

dnfw:
	@echo ""
	@echo "Downloading Firmwares"
	@echo ""
	@[ -d $(BDDIR)/rootfs/lib/firmware/rtlwifi ] || mkdir -p $(BDDIR)/rootfs/lib/firmware/rtlwifi
	@for f in $(RTLFWS); do \
		[ -f $(BDDIR)/rootfs/lib/firmware/$$f ] || ( \
			[ -d $(RTLFWTMPDIR) ] || ( \
				echo "" ;                                   \
				echo "Cloning Linux-Firmware from $(RTLFWLOC)" ; \
				echo "" ;                                   \
				mkdir -p $(RTLFWTMPDIR);                    \
				$(eval __CURDIR:=${PWD})					\
				cd $(RTLFWTMPDIR);                          \
				git clone --single-branch $(RTLFWLOC) $(RTLFWTMPDIR);  \
				cd $(__CURDIR);                             \
				git checkout $(RTLFWTAG) > /dev/null        \
			) ;                                             \
			cp -rf $(RTLFWTMPDIR)/$$f $(BDDIR)/rootfs/lib/firmware/$${f%/*} \
		) ; \
	done
	@\rm -rf $(RTLFWTMPDIR)
	@[ -d $(BDDIR)/rootfs/lib/firmware/brcm ] || mkdir -p $(BDDIR)/rootfs/lib/firmware/brcm 
	@for f in $(BRCMFWS); do \
		[ -f $(BDDIR)/rootfs/lib/firmware/$$f ] || ( \
			[ -d $(BRCMFWTMPDIR) ] || ( \
				echo "" ;                                 \
				echo "Copyiing Linux-Firmware from $(BRCMFWLOC)" ; \
				echo "" ;                                \
				mkdir -p $(BRCMFWTMPDIR);                \
				cd $(BRCMFWTMPDIR);                      \
				wget $(BRCMFWLOC);                       \
				ar x $(BRCMFWFN) ;                       \
				tar xvf data.tar.xz ;                    \
				cd $(BDDIR)/kernel                       \
			) ;                                          \
			cp -rf $(BRCMFWTMPDIR)/lib/firmware/$$f $(BDDIR)/rootfs/lib/firmware/$${f%/*} \
		) ; \
	done
	@\rm -rf $(BRCMFWTMPDIR)
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
				echo "Copyiing Linux-Firmware from $(CRDAFWLOC)" ; \
				echo "" ;                                \
				mkdir -p $(CRDAFWTMPDIR);                \
				$(eval __CURDIR:=${PWD})		         \
				cd $(CRDAFWTMPDIR);                      \
				git clone --single-branch $(CRDAFWLOC);  \
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

