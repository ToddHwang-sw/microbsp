DIR=freetype-2.11.0
LOC=https://sourceforge.net/projects/freetype/files/freetype2/2.11.0

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_MAKEOPTS += V=1 

MICB_DEPENDS = zlib libpng libpng16

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


