DIR=bzip2
LOC=https://github.com/opencor

include $(TOPDIR)/scripts/env.mk

CROSS_USER_CFLAGS += -fPIC

MICB_CONFIGURE_MAKEOPTS += PREFIX=$(MICB_INSTALL_PLACE) $(COMP_CMDS)

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare:
	@$(MICB_DUP_SOURCES)

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); \
			make $(MICB_CONFIGURE_MAKEOPTS) libbz2.a bzip2 bzip2recover ; \
			make $(MICB_CONFIGURE_MAKEOPTS) -f Makefile-libbz2_so )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); \
			make $(MICB_CONFIGURE_MAKEOPTS) $@ ; \
			make $(MICB_CONFIGURE_MAKEOPTS) -f Makefile-libbz2_so $@ )


