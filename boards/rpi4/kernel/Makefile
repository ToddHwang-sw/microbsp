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
EXTMDIRS=\
		rtl88x2bu

##
## Broadcom WiFi FWs required.... 
## Cypress  Power regulartor FWs also required....
##
##    https://www.linuxquestions.org/questions/slackware-arm-108/raspberry-pi-3-b-wifi-nic-not-found-4175627137/
##
BRCMFWDIR=brcm
CYPRESSFWDIR=cypress
BRCMFWFN=firmware-brcm80211_20210315-3+rpt7_all.deb
BRCMFWLOC=https://archive.raspberrypi.org/debian/pool/main/f/firmware-nonfree/$(BRCMFWFN)
BRCMFWS=\
	$(BRCMFWDIR)/bcm43xx-0.fw \
	$(BRCMFWDIR)/bcm43xx_hdr-0.fw \
	$(BRCMFWDIR)/brcmfmac43455-sdio.bin \
	$(BRCMFWDIR)/brcmfmac43455-sdio.txt \
	$(BRCMFWDIR)/brcmfmac43455-sdio.raspberrypi,3-model-b-plus.txt \
	$(BRCMFWDIR)/brcmfmac43455-sdio.clm_blob \
	$(CYPRESSFWDIR)/cyfmac43455-sdio-standard.bin \
	$(CYPRESSFWDIR)/cyfmac43455-sdio.clm_blob
BRCMFWTMPDIR=\
	/tmp/___rpi_linuxswtree_linux-broadcom_firmware___

##
## Realtek WiFi FWs required....
##
LINFWNM=linux-firmware
RTKWIFIDIR=rtlwifi
RTKFWLOC=git://git.kernel.org/pub/scm/linux/kernel/git/firmware/$(LINFWNM).git 
RTKFWTAG=tags/20191215
RTKFWS=\
	$(RTKWIFIDIR)/rtl8192eu_ap_wowlan.bin \
	$(RTKWIFIDIR)/rtl8192eu_nic.bin \
	$(RTKWIFIDIR)/rtl8192eu_wowlan.bin
RTKFWTMPDIR=\
	/tmp/___rpi_linuxswtree_linux-realtek_firmware____

##
## Regulatory FWs
##
CRDAFWNM=wireless-regdb
CRDAFWLOC=git://git.kernel.org/pub/scm/linux/kernel/git/sforshee/$(CRDAFWNM).git
CRDAFW0S=\
	regulatory.bin \
	sforshee.x509.pem \
	sforshee.key.pub.pem
CRDAFW1S=\
	regulatory.db \
	regulatory.db.p7s
CRDAFWTMPDIR=\
	/tmp/___rpi_linuxswtree_linux-crda_firmware____
CRDATMPTMP=\
	/tmp/___rpi_linuxswtree_linux-crda_signature___

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || ( \
		[ -f source/$(FILENM) ] || ( cd source; $(WGETCMD) $(LOC) ) )
	@[ -d $(BUILDDIR)/$(KERNELVER) ] || (                          \
		unzip source/$(FILENM) -d $(BUILDDIR) ;                    \
		cd $(BUILDDIR)/$(KERNELVER);                               \
		cat ../../../patch/patch-$(BRANCH)  | patch -p 1;          \
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
	@echo ""
	@echo "Downloading Firmwares"
	@echo ""
	@[ -d $(BDDIR)/rootfs/lib/firmware/$(RTKWIFIDIR) ] || mkdir -p $(BDDIR)/rootfs/lib/firmware/$(RTKWIFIDIR)
	@for f in $(RTKFWS); do \
		[ -f $(BDDIR)/rootfs/lib/firmware/$$f ] || ( \
			[ -d $(RTKFWTMPDIR) ] || ( \
				echo "" ;                                   \
				echo "Cloning $(LINFWNM) from $(RTKFWLOC)" ;\
				echo "" ;                                   \
				mkdir -p $(RTKFWTMPDIR);                    \
				cd $(RTKFWTMPDIR);                          \
				$(TOPDIR)/scripts/gitclone.sh "" "$(RTKFWLOC)" "" "$(RTKFWTAG)" ;  \
			) ;                                             \
			cp -rf $(RTKFWTMPDIR)/$(LINFWNM)/$$f $(BDDIR)/rootfs/lib/firmware/$${f%/*} \
		) ; \
	done
	@\rm -rf $(RTKFWTMPDIR)
	@[ -d $(BDDIR)/rootfs/lib/firmware/$(BRCMFWDIR) ] || ( \
			mkdir -p $(BDDIR)/rootfs/lib/firmware/$(BRCMFWDIR) ; \
			mkdir -p $(BDDIR)/rootfs/lib/firmware/$(CYPRESSFWDIR) )
	@echo ""
	@echo "Moving $(BRCMFWDIR) binary file "
	@echo ""
	@for f in $(BRCMFWS); do \
		[ -f $(BDDIR)/rootfs/lib/firmware/$$f ] || ( \
			[ -d $(BRCMFWTMPDIR) ] || ( \
				echo "" ;                                \
				echo "Copying $(BRCMFWLOC)" ;            \
				echo "" ;                                \
				mkdir -p $(BRCMFWTMPDIR);                \
				cd $(BRCMFWTMPDIR);                      \
				$(WGETCMD) $(BRCMFWLOC);                 \
				ar x $(BRCMFWFN) ;                       \
				tar xvf data.tar.xz ;                    \
				cd $(BDDIR)/kernel                       \
			) ;                                          \
			cp -rf $(BRCMFWTMPDIR)/lib/firmware/$$f $(BDDIR)/rootfs/lib/firmware/$${f%/*} ; \
		) ; \
	done
	@echo ""
	@echo "Moving $(CYPRESSFWDIR) binary file "
	@echo ""
	@mv $(BDDIR)/rootfs/lib/firmware/$(CYPRESSFWDIR)/cyfmac43455-sdio-standard.bin \
		$(BDDIR)/rootfs/lib/firmware/$(CYPRESSFWDIR)/cyfmac43455-sdio.bin
	@\rm -rf $(BRCMFWTMPDIR)
	@[ -d $(BDDIR)/rootfs/lib/crda ] || mkdir -p $(BDDIR)/rootfs/lib/crda
	@for f in $(CRDAFW0S); do \
		[ -f $(BDDIR)/rootfs/lib/crda/$$f ] || ( \
			[ -d $(CRDAFWTMPDIR) ] || ( \
				echo "" ;                                 \
				echo "Copyiing $(CRDAFWNM) from $(CRDAFWLOC)" ; \
				echo "" ;                                \
				mkdir -p $(CRDAFWTMPDIR);                \
				cd $(CRDAFWTMPDIR);                      \
				$(TOPDIR)/scripts/gitclone.sh "--single-branch" "$(CRDAFWLOC)" ; \
				cd $(BDDIR)/kernel                       \
			) ;                                          \
			cp -rf $(CRDAFWTMPDIR)/$(CRDAFWNM)/$$f $(BDDIR)/rootfs/lib/crda/$$f \
		) ; \
	done
	@[ -d $(BDDIR)/rootfs/lib/firmware ] || mkdir -p $(BDDIR)/rootfs/lib/firmware
	@for f in $(CRDAFW1S); do \
		[ -f $(BDDIR)/rootfs/lib/firmware/$$f ] || ( \
			[ -d $(CRDAFWTMPDIR) ] || ( \
				echo "" ;                                 \
				echo "Copyiing $(CRDAFWNM) from $(CRDAFWLOC)" ; \
				echo "" ;                                \
				mkdir -p $(CRDAFWTMPDIR);                \
				cd $(CRDAFWTMPDIR);                      \
				$(TOPDIR)/scripts/gitclone.sh "--single-branch" "$(CRDAFWLOC)";  \
				cd $(BDDIR)/kernel                       \
			) ;                                          \
			cp -rf $(CRDAFWTMPDIR)/$(CRDAFWNM)/$$f $(BDDIR)/rootfs/lib/firmware/$$f \
		) ; \
	done
	@\rm -rf $(CRDAFWTMPDIR)

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

