DIR=lz4
LOC=https://github.com/lz4

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_MAKEOPTS += $(COMP_CMDS) PREFIX=

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare:
	@$(MICB_DUP_SOURCES)

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) allmost )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

