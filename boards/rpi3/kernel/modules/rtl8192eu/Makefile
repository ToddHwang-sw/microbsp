DIR=rtl8192eu
LOC=https://github.com/clnhub/rtl8192eu-linux.git

RTLMAKE=$(LINUXMAKE) KSRC=$(KERNSRC) CROSS_COMPILE=$(CROSS_COMP_PREFIX) ARCH=arm64 
KERNVER=`cat $(KERNSRC)/include/config/kernel.release`

prepare:
	@[ -d $(DIR) ] || ( \
		git clone $(LOC) $(DIR); \
		cd $(DIR); \
			sed -i "s/CONFIG_PLATFORM_ARM_AARCH64 = n/CONFIG_PLATFORM_ARM_AARCH64 = y/" Makefile ; \
			sed -i "s/CONFIG_POWER_SAVING = y/CONFIG_POWER_SAVING = n/" Makefile ; \
			sed -i "s/CONFIG_BT_COEXIST = y/CONFIG_BT_COEXIST = n/" Makefile ; \
			sed -i "s/ONFIG_PLATFORM_I386_PC = y/ONFIG_PLATFORM_I386_PC = n/" Makefile )

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

