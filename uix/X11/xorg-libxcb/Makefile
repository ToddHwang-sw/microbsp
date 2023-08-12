DIR=libxcb-1.14
LOC=https://xcb.freedesktop.org/dist

include $(TOPDIR)/scripts/x11uixenv.mk

MICB_CONFIGURE_MAKEOPTS += \
	XCBPROTO_XCBINCLUDEDIR=$(X11INSTDIR)/share/xcb \
	XCBPROTO_XCBPYTHONDIR=$(X11INSTDIR)/lib/python3.8/site-packages

MICB_DEPENDS = xproto xau

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz ; $(MICB_PATCH) )

prepare:
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


