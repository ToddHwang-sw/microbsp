DIR=jpegsrc.v9e
LOC=http://www.ijg.org/files
REALDIR=jpeg-9e

include $(TOPDIR)/scripts/uixenv.mk

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@[ -d $(MICBSRC)/$(DIR) ] || mv $(MICBSRC)/$(REALDIR) $(MICBSRC)/$(DIR)

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )
