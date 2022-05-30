DIR=busybox-1.28.0
LOC=https://busybox.net/downloads

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_MAKEOPTS=\
	CROSS_COMPILE="$(CROSS_COMP_PREFIX)" CFLAGS="$(CROSS_COMP_FLAGS)" LDFLAGS="-L$(destination)/lib -L$(destination)/usr/lib64" \
	CONFIG_PREFIX=$(destination) \
	V=1

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.bz2 ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.bz2; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		echo "CROSS_COMPILE=$(CROSS_COMP_PREFIX)"      > .compile_cmds; \
		echo "CFLAGS=\"$(CROSS_COMP_FLAGS)\""         >> .compile_cmds; \
		cp  ../../../patch/config .config;				\
		make KBUILD_SRC=../../../$(MICBSRC)/$(DIR) -f ../../../$(MICBSRC)/$(DIR)/Makefile oldconfig

all install clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


