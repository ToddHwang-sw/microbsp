DIR=zstd
LOC=-b dev https://github.com/facebook

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_MAKEOPTS += VERBOSE=1 $(COMP_CMDS) OS=linux prefix=/usr

MICB_DEPENDS = zlib

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || ( \
		[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR); \
		$(TOPDIR)/scripts/setupsrcs.sh $(BUILDDIR)/$(DIR) $(LOC)/$(DIR).git; \
		[ ! -f $(MICB_PATCH_FN) ] || ( \
			cd $(BUILDDIR)/$(DIR); cat ../../../$(MICB_PATCH_FN) | patch -p 1 ) \
	)

prepare:

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) allmost )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

