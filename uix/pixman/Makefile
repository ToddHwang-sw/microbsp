DIR=pixman
LOC=https://github.com/freedesktop

include $(TOPDIR)/scripts/uixenv.mk

MICB_DEPENDS = libpng libpng16

MICB_CONFIGURE_MAKEOPTS += V=1

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


