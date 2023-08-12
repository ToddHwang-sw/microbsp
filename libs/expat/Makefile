DIR=expat-2.2.9
LOC=https://github.com/libexpat/libexpat/releases/download/R_2_2_9

include $(TOPDIR)/scripts/env.mk

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.bz2 ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.bz2; $(MICB_PATCH) )
	@[ -d $(MICBSRC)/$(DIR) ] || mv $(MICBSRC)/$(REALDIR) $(MICBSRC)/$(DIR)

prepare: 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

