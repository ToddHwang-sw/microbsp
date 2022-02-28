DIR=tcpdump-4.9.3
LOC=https://www.tcpdump.org/release

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	LIBS="-lnl-3 -lnl-genl-3" \
	PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH)

MICB_CONFIGURE_OPTS += \
	--without-smi 

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

