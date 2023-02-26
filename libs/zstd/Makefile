DIR=zstd
LOC=-b dev https://github.com/facebook

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_MAKEOPTS += \
	  VERBOSE=1 $(COMP_CMDS) OS=linux

MICB_DEPENDS = zlib

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(BUILDDIR)/$(DIR)/.git ] || ( \
			cd $(BUILDDIR) ; \
			git clone $(LOC)/$(DIR).git ;   \
			[ ! -f ../../patch/patch ] || ( \
				cd $(DIR); cat ../../../patch/patch | patch -p 1 ) \
			)

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) allmost )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

