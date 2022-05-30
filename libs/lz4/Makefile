DIR=lz4
LOC=https://github.com/lz4

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_MAKEOPTS += $(COMP_CMDS) PREFIX=

prepare: 
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(BUILDDIR)/$(DIR)/.git ] || ( cd $(BUILDDIR); git clone $(LOC)/$(DIR).git )

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) allmost )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

