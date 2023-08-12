DIR=strace-5.3
LOC=https://strace.io/files/5.3

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH)

MICB_CONFIGURE_OPTS += \
	--enable-mpers=no

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.xz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.xz; $(MICB_PATCH) )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

