DIR=libcap-2.24
LOC=https://www.kernel.org/pub/linux/libs/security/linux-privs/libcap2

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_MAKEOPTS += $(COMP_CMDS) BUILD_CC=gcc LD="$(CROSS_COMP_PREFIX)gcc -shared" WARNINGS= 

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.xz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.xz ; $(MICB_PATCH) )
	@[ -f $(BUILDDIR)/$(DIR)/Makefile ] || (          \
		cd $(BUILDDIR)/$(DIR) ;                       \
		cp -rf ../../../$(MICBSRC)/$(DIR)/* . )

prepare: 

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


