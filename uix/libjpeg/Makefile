DIR=jpegsrc.v9d
LOC=http://www.ijg.org/files
REALDIR=jpeg-9d

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_MAKEOPTS=\
		V=1 \
		prefix=$(destination)

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@[ -d $(MICBSRC)/$(DIR) ] || mv $(MICBSRC)/$(REALDIR) $(MICBSRC)/$(DIR)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


