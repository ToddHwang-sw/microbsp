DIR=libXmu-1.1.3
LOC=https://www.x.org/releases/individual/lib

include $(TOPDIR)/scripts/x11uixenv.mk

MICB_DEPENDS = xt x11-xcb uuid

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.bz2 ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.bz2 ; $(MICB_PATCH) )
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


