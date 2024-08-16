DIR=zstd
LOC=-b dev https://github.com/facebook

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_MAKEOPTS += VERBOSE=1 $(COMP_CMDS) OS=linux prefix=/usr

MICB_DEPENDS = zlib

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare:
	@$(MICB_DUP_SOURCES)

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) allmost )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

