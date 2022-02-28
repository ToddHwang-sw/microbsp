DIR=webui

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_PRG = ./configure 

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@$(MICB_DUP_SOURCES)
	@$(MICB_CONFIGURE_BUILDDIR_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

wipeout:

