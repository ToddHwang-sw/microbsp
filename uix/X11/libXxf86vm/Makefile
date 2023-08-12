DIR=libXxf86vm
LOC=https://gitlab.freedesktop.org/xorg/lib

include $(TOPDIR)/scripts/x11uixenv.mk

MICB_DEPENDS = x11 xext

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ -d $(MICBSRC)/$(DIR)/m4  ] || ( mkdir -p $(MICBSRC)/$(DIR)/m4 )

prepare:
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


