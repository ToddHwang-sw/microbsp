DIR=$(BUSYBOX)
LOC=https://busybox.net/downloads

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_MAKEOPTS=\
	CROSS_COMPILE="$(CROSS_COMP_PREFIX)" CFLAGS="$(CROSS_COMP_FLAGS) -DLONG_BIT=32" LDFLAGS="-L$(destination)/lib -L$(destination)/usr/lib64" \
	CONFIG_PREFIX=$(destination) \
	V=1

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.bz2 ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.bz2; $(MICB_PATCH) )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		echo "CROSS_COMPILE=$(CROSS_COMP_PREFIX)"      > .compile_cmds; \
		echo "CFLAGS=\"$(CROSS_COMP_FLAGS)\""         >> .compile_cmds; \
		cp  ../../../patch/config           .config;	                \
		[ ! -f ../../../patch/config.$(TBOARD) ] || cp  ../../../patch/config.$(TBOARD) .config;	\
		make KBUILD_SRC=../../../$(MICBSRC)/$(DIR) -f ../../../$(MICBSRC)/$(DIR)/Makefile oldconfig

all install clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


