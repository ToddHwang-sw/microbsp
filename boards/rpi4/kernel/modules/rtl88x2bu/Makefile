DIR=88x2bu-20210702
LOC=https://github.com/morrownr

RTLMAKE=$(LINUXMAKE) KSRC=$(KERNSRC) CROSS_COMPILE=$(CROSS_COMP_PREFIX) ARCH=arm64 V=1
KERNVER=`cat $(KERNSRC)/include/config/kernel.release`

prepare:
	[ -d $(DIR) ] || ( \
		git clone $(LOC)/$(DIR).git ; \
		cd $(DIR); \
		[ ! -f ../patch/patch ] || cat ../patch/patch | patch -p 1 )

all clean:
	@[ ! -d $(DIR) ] || \
		$(RTLMAKE) -C $(DIR) $@

install:
	@[ ! -d $(DIR) ] || ( \
		$(RTLMAKE) \
			MODDESTDIR=$(INSTALLDIR)/lib/modules/$(KERNVER)/kernel/drivers/net/wireless \
			-C $(KERNSRC) \
			M=`pwd`/$(DIR) \
			INSTALL_MOD_PATH=$(INSTALLDIR) \
			modules_install ;  \
		find $(INSTALLDIR)/lib/ -type f -name "*.ko" -print | \
			sh $(TOPDIR)/scripts/modules-alias.sh $(INSTALLDIR)/etc/modules.conf )
