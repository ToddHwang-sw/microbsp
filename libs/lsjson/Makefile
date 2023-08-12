DIR=lsjson

include $(TOPDIR)/scripts/env.mk

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)

prepare: 
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

wipeout:
	@cd $(MICBSRC)/$(DIR); \
		\rm -rf aclocal.m4 autom4te.cache/ compile config.* configure depcomp install-sh  Makefile.in  missing ltmain.sh

